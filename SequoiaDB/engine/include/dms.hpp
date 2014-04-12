/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dms.hpp

   Descriptive Name = Data Management Service Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   dms Reccord ID (RID).

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMS_HPP_
#define DMS_HPP_
#include "core.hpp"
#include "oss.hpp"

namespace engine
{
#define DMS_COLLECTION_SPACE_NAME_SZ      127
// page length can be 4/8/16/32/64K
// Note that windows memory allocation granulartiy is 64K, so we need to make
// sure each segment must be multiple of 64K
#if defined (_LINUX)
#define DMS_PAGE_SIZE4K        4096ll
#define DMS_PAGE_SIZE8K        8192ll
#define DMS_PAGE_SIZE16K       16384ll
#define DMS_PAGE_SIZE32K       32768ll
#define DMS_PAGE_SIZE64K       65536ll
#elif defined (_WINDOWS)
#define DMS_PAGE_SIZE4K        4096LL
#define DMS_PAGE_SIZE8K        8192LL
#define DMS_PAGE_SIZE16K       16384LL
#define DMS_PAGE_SIZE32K       32768LL
#define DMS_PAGE_SIZE64K       65536LL
#endif

#define DMS_PAGE_SIZE_DFT      DMS_PAGE_SIZE64K
#define DMS_PAGE_SIZE_MAX      DMS_PAGE_SIZE64K

// the maximum number of pages * size for the storage unit
// this number does NOT count metadata
// max SU size:
// 4K: 512GB
// 8K: 1TB
// 16K: 2TB
// 32K: 4TB
// 64K: 8TB
// Note this number is 2^28
#define DMS_MAX_PG             (128*1024*1024)
#define DMS_MAX_SZ(x)          (((UINT64)DMS_MAX_PG)*(x))

// fixed segment size 128MB
#define DMS_SEGMENT_SZ         (128*1024*1024)
#define DMS_SEGMENT_PG(x)      (DMS_SEGMENT_SZ/(x))
// max number of segments is max size of storage unit / size of each segment
#define DMS_MAX_SEGMENT_NUM(x) (DMS_MAX_SZ(x)/DMS_SEGMENT_SZ)

#define DMS_MAX_EXTENT_SZ      DMS_SEGMENT_SZ
#define DMS_MIN_EXTENT_SZ(x)   (x)

#define DMS_ID_KEY_NAME        "_id"

#define DMS_COLLECTION_NAME_SZ      127
#define DMS_COLLECTION_MAX_INDEX    64
#define DMS_MME_SLOTS               4096
#define DMS_COLLECTION_FULL_NAME_SZ \
   ( DMS_COLLECTION_SPACE_NAME_SZ + DMS_COLLECTION_NAME_SZ + 1 )

#define DMS_INVALID_SUID            -1
#define DMS_INVALID_CLID            ~0
#define DMS_INVALID_OFFSET          -1
#define DMS_INVALID_EXTENT          -1
#define DMS_INVALID_MBID            65535
#define DMS_INVALID_PAGESIZE        0
#define DMS_INVALID_LOGICCSID       0xffffffff

#define DMS_DATA_SU_EXT_NAME        "data"
#define DMS_INDEX_SU_EXT_NAME       "idx"

#define SDB_DMSTEMP_NAME            "SYSTEMP"
#define DMS_TEMP_NAME_PATTERN       "%s%04d"

   /*
      DMS TOOL FUNCTIONS:
   */
   enum _DMS_ACCESS_TYPE
   {
      DMS_ACCESS_TYPE_NULL  = 0,
      DMS_ACCESS_TYPE_QUERY,
      DMS_ACCESS_TYPE_FETCH,
      DMS_ACCESS_TYPE_INSERT,
      DMS_ACCESS_TYPE_UPDATE,
      DMS_ACCESS_TYPE_DELETE,
      DMS_ACCESS_TYPE_TRUNCATE,
      DMS_ACCESS_TYPE_CRT_INDEX,
      DMS_ACCESS_TYPE_DROP_INDEX
   } ;
   typedef enum _DMS_ACCESS_TYPE DMS_ACCESS_TYPE ;

   typedef SINT32 dmsStorageUnitID ;
   typedef SINT32 dmsExtentID ;
   typedef SINT32 dmsOffset ;

   /*
      _dmsRecordID defined
   */
   class _dmsRecordID : SDBObject
   {
   public :
      dmsExtentID _extent ;
      dmsOffset   _offset ;
      _dmsRecordID ()
      {
         _extent = DMS_INVALID_EXTENT ;
         _offset = DMS_INVALID_OFFSET ;
      }
      _dmsRecordID ( dmsExtentID extent, dmsOffset offset )
      {
         _extent = extent ;
         _offset = offset ;
      }
      _dmsRecordID& operator=(const _dmsRecordID &rhs)
      {
         _extent=rhs._extent;
         _offset=rhs._offset;
         return *this;
      }
      BOOLEAN isNull () const
      {
         return DMS_INVALID_EXTENT == _extent ;
      }
      BOOLEAN isValid () const
      {
         return DMS_INVALID_EXTENT != _extent ;
      }
      BOOLEAN operator!=(const _dmsRecordID &rhs) const
      {
         return !(_extent == rhs._extent &&
                  _offset == rhs._offset) ;
      }
      BOOLEAN operator==(const _dmsRecordID &rhs) const
      {
         return ( _extent == rhs._extent &&
                  _offset == rhs._offset) ;
      }
      BOOLEAN operator<=(const _dmsRecordID &rhs) const
      {
         return compare(rhs)<=0 ;
      }
      BOOLEAN operator<(const _dmsRecordID &rhs) const
      {
         return compare(rhs)<0 ;
      }
      // <0 if current object sit before argment rid
      // =0 means extent/offset are the same
      // >0 means current obj sit after argment rid
      INT32 compare ( const _dmsRecordID &rhs ) const
      {
         if (_extent != rhs._extent)
            return _extent - rhs._extent ;
         return _offset - rhs._offset ;
      }
      void reset()
      {
         _extent = DMS_INVALID_EXTENT ;
         _offset = DMS_INVALID_OFFSET ;
      }
      void resetMax()
      {
         _extent = 0x7FFFFFFF ;
         _offset = 0x7FFFFFFF ;
      }
      void resetMin()
      {
         _extent = 0 ;
         _offset = 0 ;
      }
   } ;
   typedef class _dmsRecordID dmsRecordID ;

   // helper function, check DMS/IXM object name validity
   BOOLEAN  dmsIsSysCSName ( const CHAR *collectionSpaceName ) ;
   INT32    dmsCheckCSName ( const CHAR *collectionSpaceName,
                             BOOLEAN sys = FALSE ) ;

   BOOLEAN  dmsIsSysCLName ( const CHAR *collectionName ) ;
   INT32    dmsCheckCLName ( const CHAR *collectionName,
                             BOOLEAN sys = FALSE ) ;
   INT32    dmsCheckFullCLName ( const CHAR *collectionName,
                                 BOOLEAN sys = FALSE ) ;
   BOOLEAN  dmsIsSysIndexName ( const CHAR *indexName ) ;
   INT32    dmsCheckIndexName ( const CHAR *indexName,
                                BOOLEAN sys = FALSE ) ;

}

#endif /* DMS_HPP_ */

