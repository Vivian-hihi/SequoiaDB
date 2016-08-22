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

   Source File Name = dmsRecord.hpp

   Descriptive Name = Data Management Service Record Header

   When/how to use: this program may be used on binary and text-formatted
   versions of data management component. This file contains structure for
   data record, including normal record and deleted record.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef DMSRECORD_HPP_
#define DMSRECORD_HPP_

#include "dms.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"

namespace engine
{
   // 2^24 = 16MB, shouldn't be changed without completely redesign the
   // dmsRecord structure this size includes metadata header

   #define DMS_RECORD_MAX_SZ           0x01000000

   // since DPS may need extra space for header, let's make sure the max size of
   // user record ( the ones need to log ) cannot exceed 16M-4K

   #define DMS_RECORD_USER_MAX_SZ      (DMS_RECORD_MAX_SZ-4096)

   /*
      _dmsRecordData define
   */
   class _dmsRecordData : public SDBObject
   {
      public:
         _dmsRecordData()
         {
            reset() ;
         }
         _dmsRecordData( const CHAR *data, UINT32 len,
                         BOOLEAN isCompress = FALSE,
                         BOOLEAN isOrgData = TRUE )
         {
            _data = data ;
            _len = len ;
            _isCompress = isCompress ;

            if ( isOrgData )
            {
               _orgData = data ;
               _orgLen = len ;
            }
            else
            {
               _orgData = NULL ;
               _orgLen = 0 ;
            }
         }
         ~_dmsRecordData()
         {
         }

         BOOLEAN isEmpty() const { return !_data || 0 == _len ; }

         const CHAR* data() const { return _data ; }
         UINT32 len() const { return _len ; }

         const CHAR* orgData() const { return _orgData ; }
         UINT32 orgLen() const { return _orgLen ; }

         BOOLEAN isCompressed() const { return _isCompress ; }
         FLOAT32 getCompressRatio() const
         {
            if ( _isCompress && _len > 0 && _orgLen > 0 )
            {
               return ( (FLOAT32)_len ) / (FLOAT32)_orgLen ;
            }
            return 1.0 ;
         }

         void setData( const CHAR *data, UINT32 len,
                       BOOLEAN compressed = FALSE,
                       BOOLEAN isOrgData = TRUE )
         {
            _data = data ;
            _len = len ;
            _isCompress = compressed ;

            if ( isOrgData )
            {
               _orgData = data ;
               _orgLen = len ;
            }
         }
         void setOrgData( const CHAR *orgData, UINT32 orgLen )
         {
            _orgData = orgData ;
            _orgLen = orgLen ;
         }
         void setCompress( BOOLEAN compressed )
         {
            _isCompress = compressed ;
         }

         void reset()
         {
            _data = NULL ;
            _len = 0 ;
            _isCompress = FALSE ;
            _orgData = NULL ;
            _orgLen = 0 ;
         }
         void resetOrgData()
         {
            _orgData = NULL ;
            _orgLen = 0 ;
         }

      private:
         const CHAR     *_data ;
         UINT32         _len ;

         BOOLEAN        _isCompress ;

         const CHAR     *_orgData ;
         UINT32         _orgLen ;

   } ;
   typedef _dmsRecordData dmsRecordData ;

   /*
      Record Flag define:
   */
   // 0~3 bit for STATE
   #define DMS_RECORD_FLAG_NORMAL            0x00
   #define DMS_RECORD_FLAG_OVERFLOWF         0x01
   #define DMS_RECORD_FLAG_OVERFLOWT         0x02
   #define DMS_RECORD_FLAG_DELETED           0x04
   // 4~7 bit for ATTR
   #define DMS_RECORD_FLAG_COMPRESSED        0x10
   // some one wait X-lock, the last one who get X-lock will delete the record
   #define DMS_RECORD_FLAG_DELETING          0x80

   #define DMS_RECORD_METADATA_SZ   sizeof(_dmsRecord)
   /*
      _dmsRecord defined
   */
   class _dmsRecord : public SDBObject
   {
   public:
      union
      {
         CHAR     _recordHead[4] ;     // 1 byte flag, 3 bytes length - 1
         UINT32   _flag_and_size ;
      }                 _head ;
      dmsOffset         _myOffset ;
      dmsOffset         _previousOffset ;
      dmsOffset         _nextOffset ;
      /*
         Follow _nextOffset is:
            if overflow, is overflow rid(8bytes)
            else, is the data length(4 bytes).
                In compressed: 4bytes + data
                uncompressed:  data( the first 4bytes is bson size )
      */

      /*
         Get Functions
      */
      dmsOffset getMyOffset() const
      {
         return _myOffset ;
      }
      dmsOffset getPrevOffset() const
      {
         return _previousOffset ;
      }
      dmsOffset getNextOffset() const
      {
         return _nextOffset ;
      }
      BYTE getFlag() const
      {
         return (BYTE)_head._recordHead[ 0 ] ;
      }
      BYTE getState() const
      {
         return (BYTE)(getFlag() & 0x0F) ;
      }
      BYTE getAttr() const
      {
         return (BYTE)(getFlag() & 0xF0) ;
      }
      BOOLEAN isOvf() const
      {
         return  DMS_RECORD_FLAG_OVERFLOWF == getState() ;
      }
      BOOLEAN isOvt() const
      {
         return DMS_RECORD_FLAG_OVERFLOWT == getState() ;
      }
      BOOLEAN isDeleted() const
      {
         return DMS_RECORD_FLAG_DELETED == getState() ;
      }
      BOOLEAN isNormal() const
      {
         return DMS_RECORD_FLAG_NORMAL == getState() ;
      }
      BOOLEAN isCompressed() const
      {
         return getAttr() & DMS_RECORD_FLAG_COMPRESSED ;
      }
      BOOLEAN isDeleting() const
      {
         return getAttr() & DMS_RECORD_FLAG_DELETING ;
      }
      dmsRecordID getOvfRID() const
      {
         if ( isOvf() )
         {
            return *( const dmsRecordID* )( (const CHAR*)this +
                                            sizeof( _dmsRecord ) ) ;
         }
         return dmsRecordID() ;
      }
      UINT32 getSize() const
      {
#if defined (SDB_BIG_ENDIAN)
         return (((*((const UINT32*)this))&0x00FFFFFF)+1) ;
#else
         return (((*((const UINT32*)this))>>8)+1) ;
#endif // SDB_BIG_ENDIAN
      }
      UINT32 getDataLength() const
      {
         return *(const UINT32*)((const CHAR*)this+DMS_RECORD_METADATA_SZ) ;
      }
      /*
         Get disk data only, if compressed, not uncompressed
      */
      const CHAR* getData() const
      {
         return isCompressed() ?
            ((const CHAR*)this+sizeof(UINT32)+DMS_RECORD_METADATA_SZ) :
            ((const CHAR*)this+DMS_RECORD_METADATA_SZ) ;
      }

      /*
         Set Functions
      */
      void setMyOffset( dmsOffset offset )
      {
         _myOffset = offset ;
      }
      void setPrevOffset( dmsOffset offset )
      {
         _previousOffset = offset ;
      }
      void setNextOffset( dmsOffset offset )
      {
         _nextOffset = offset ;
      }
      void  setFlag( BYTE flag )
      {
         _head._recordHead[ 0 ] = (CHAR)flag ;
      }
      void  setState( BYTE state )
      {
         _head._recordHead[ 0 ] = (CHAR)((state&0x0F)|getAttr()) ;
      }
      void setAttr( BYTE attr )
      {
         _head._recordHead[ 0 ] = (BYTE)((attr&0xF0)|getState()) ;
      }
      void unsetAttr( BYTE attr )
      {
         _head._recordHead[ 0 ] &= (BYTE)(~(attr&0xF0)) ;
      }
      void resetAttr()
      {
         unsetAttr( 0xF0 ) ;
      }
      void setOvf()
      {
         setState( DMS_RECORD_FLAG_OVERFLOWF ) ;
      }
      void setOvt()
      {
         setState( DMS_RECORD_FLAG_OVERFLOWT ) ;
      }
      void setNormal()
      {
         setState( DMS_RECORD_FLAG_NORMAL ) ;
      }
      void setDeleted()
      {
         setState( DMS_RECORD_FLAG_DELETED ) ;
      }
      void setCompressed()
      {
         setAttr( DMS_RECORD_FLAG_COMPRESSED ) ;
      }
      void unsetCompressed()
      {
         unsetAttr( DMS_RECORD_FLAG_COMPRESSED ) ;
      }
      void setDeleting()
      {
         setAttr( DMS_RECORD_FLAG_DELETING ) ;
      }
      void unsetDeleting()
      {
         unsetAttr( DMS_RECORD_FLAG_DELETING ) ;
      }
      void setOvfRID( const dmsRecordID &rid )
      {
         *((dmsRecordID*)((CHAR*)this+DMS_RECORD_METADATA_SZ)) = rid ;
      }
      void  setSize( UINT32 size )
      {
#if defined (SDB_BIG_ENDIAN)
      (*((UINT32*)this) = ((UINT32)getFlag()<<24) |
                          ((UINT32)((size)-1)&0x00FFFFFF)) ;
#else
      (*((UINT32*)this) = (UINT32)getFlag() | ((UINT32)((size)-1)<<8)) ;
#endif
      }
      /*
         Copy the data to disk directly
      */
      void  setData( const dmsRecordData &data )
      {
         if ( data.isEmpty() )
         {
            return ;
         }
         if ( data.isCompressed() )
         {
            setCompressed() ;
            *(UINT32*)((CHAR*)this+DMS_RECORD_METADATA_SZ) = data.len() ;
            ossMemcpy( (CHAR*)this+DMS_RECORD_METADATA_SZ+sizeof(UINT32),
                       data.data(), data.len() ) ;
         }
         else
         {
            unsetCompressed() ;
            ossMemcpy( (CHAR*)this+DMS_RECORD_METADATA_SZ,
                       data.data(), data.len() ) ;
         }
      }
   } ;
   typedef _dmsRecord dmsRecord ;

   #define DMS_RECORD_GETFLAG(record)        (*((CHAR*)(record)))
   #define DMS_RECORD_GETSTATE(record)       (*((CHAR*)(record))&0x0F)
   #define DMS_RECORD_GETATTR(record)        (*((CHAR*)(record))&0xF0)

#if defined (SDB_BIG_ENDIAN)
   #define DMS_RECORD_GETSIZE(record)  (((*((UINT32*)(record)))&0x00FFFFFF)+1)
#else
   #define DMS_RECORD_GETSIZE(record)  (((*((UINT32*)(record)))>>8)+1)
#endif

   #define DMS_RECORD_GETNEXTOFFSET(record)  \
      (((dmsRecord*)(record))->_nextOffset)

   #define DMS_RECORD_GETDATA(record)                                         \
      ( OSS_BIT_TEST(DMS_RECORD_GETATTR(record),DMS_RECORD_FLAG_COMPRESSED) ? \
       (CHAR*)((CHAR*)(record)+sizeof(INT32)+DMS_RECORD_METADATA_SZ) :        \
       (CHAR*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ) )

   // Get OVF RID
   #define DMS_RECORD_GETOVF(record)         \
      *(dmsRecordID*)((char*)(record)+DMS_RECORD_METADATA_SZ)

   // 4 bytes after metadata are record length, for non compressed
   // and compressed
   #define DMS_RECORD_GETDATALEN(record)     \
      *(INT32*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ)

   // Extract Data
   #define DMS_RECORD_EXTRACTDATA( recordPtr, retPtr, compressorEntry ) \
   do {                                                                 \
         if ( !OSS_BIT_TEST( DMS_RECORD_GETATTR(recordPtr),             \
                             DMS_RECORD_FLAG_COMPRESSED ) )             \
         {                                                              \
            (retPtr) = (ossValuePtr)(DMS_RECORD_GETDATA(recordPtr)) ;   \
         }                                                              \
         else                                                           \
         {                                                              \
            INT32 uncompLen = 0 ;                                       \
            rc = dmsUncompress( cb, compressorEntry,                    \
                                DMS_RECORD_GETDATA(recordPtr),          \
                                DMS_RECORD_GETDATALEN(recordPtr),       \
                                (const CHAR**)&(retPtr), &uncompLen ) ; \
            PD_RC_CHECK ( rc, PDERROR,                                  \
                          "Failed to uncompress record, rc = %d", rc ); \
            PD_CHECK ( uncompLen == *(INT32*)(retPtr),                  \
                       SDB_CORRUPTED_RECORD, error, PDERROR,            \
                       "uncompressed length %d does not match real "    \
                       "len %d", uncompLen, *(INT32*)(retPtr) ) ;       \
         }                                                              \
      } while ( FALSE )

   #define DMS_RECORD_SETFLAG(record,flag)   (*((CHAR*)(record))=(CHAR)(flag))
   #define DMS_RECORD_SETSTATE(record,state)  \
      (*((CHAR*)(record))=(CHAR)(state&0x0F)|(*((CHAR*)(record))&0xF0))
   #define DMS_RECORD_SETATTR(record,attr)   \
      (*((CHAR*)(record))=(CHAR)(attr&0xF0)|(*((CHAR*)(record))))
   #define DMS_RECORD_UNSETATTR(record,attr) \
      (*((CHAR*)(record))=(CHAR)(~(attr&0xF0))&(*((CHAR*)(record))))
   #define DMS_RECORD_RESETATTR(record)      DMS_RECORD_UNSETATTR(record,0xF0)

#if defined (SDB_BIG_ENDIAN)
   #define DMS_RECORD_SETSIZE(record,size)   \
      (*((UINT32*)(record))=(DMS_RECORD_GETFLAG(record)<<24)|\
      ((UINT32)((size)-1)&0x00FFFFFF))
#else
   #define DMS_RECORD_SETSIZE(record,size)      \
      (*((UINT32*)(record))=DMS_RECORD_GETFLAG(record)|\
      ((UINT32)((size)-1)<<8))
#endif

   #define DMS_RECORD_SETMYOFFSET(record,offset)   \
      (((dmsRecord*)(record))->_myOffset=(offset))
   #define DMS_RECORD_SETPREVOFFSET(record,offset) \
      (((dmsRecord*)(record))->_previousOffset=(offset))
   #define DMS_RECORD_SETNEXTOFFSET(record,offset) \
      (((dmsRecord*)(record))->_nextOffset=(offset))

   // SET DATA
   #define DMS_RECORD_SETDATA(record,ptr,len)                              \
     do {                                                                  \
        if ( OSS_BIT_TEST(DMS_RECORD_GETATTR(record),                      \
                          DMS_RECORD_FLAG_COMPRESSED) )                    \
        {                                                                  \
           *(INT32*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ) = len ;       \
           ossMemcpy((void*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ+       \
                      sizeof(INT32)), ((CHAR*)ptr),(len)) ;                \
        }                                                                  \
        else                                                               \
        {                                                                  \
           ossMemcpy((void*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ),      \
                     ((CHAR*)ptr),(len)) ;                                 \
        }                                                                  \
     } while (FALSE)

   // SET DATA AND OID, Can't be compressed
   #define DMS_RECORD_SETDATA_OID(record,ptr,len,oid)                      \
      do {                                                                 \
           *(INT32*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ) =             \
                     (len) + oid.size() ;                                  \
           ossMemcpy((void*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ+       \
                     sizeof(INT32)), oid.rawdata(), oid.size()) ;          \
           ossMemcpy((void*)((CHAR*)(record)+DMS_RECORD_METADATA_SZ+       \
                     sizeof(INT32)+oid.size()),                            \
                     (CHAR*)((CHAR*)(ptr)+sizeof(INT32)),                  \
                     (len)-sizeof(INT32)) ;                                \
        } while ( FALSE )


   /*
      _dmsDeletedRecord defined
   */
   class _dmsDeletedRecord : public SDBObject
   {
   public :
      union
      {
         CHAR     _recordHead[4] ;     // 1 byte flag, 3 bytes length-1
         UINT32   _flag_and_size ;
      }                 _head ;
      dmsOffset         _myOffset ;
      dmsRecordID       _next ;

      /*
         Get Functions
      */
      CHAR getFlag() const
      {
         return _head._recordHead[ 0 ] ;
      }
      BOOLEAN isDeleted() const
      {
         return DMS_RECORD_FLAG_DELETED == getFlag() ;
      }
      UINT32 getSize() const
      {
         return ((const dmsRecord*)this)->getSize() ;
      }
      dmsOffset getMyOffset() const
      {
         return _myOffset ;
      }
      dmsRecordID getNextRID() const
      {
         return _next ;
      }

      /*
         Set Functions
      */
      void setSize( UINT32 size )
      {
         return ((dmsRecord*)this)->setSize( size ) ;
      }
      void setMyOffset( dmsOffset myOffset )
      {
         _myOffset = myOffset ;
      }
      void setNextRID( const dmsRecordID &rid )
      {
         _next = rid ;
      }
      void setFlag( CHAR flag )
      {
         _head._recordHead[ 0 ] = flag ;
      }
      void setDeleted()
      {
         setFlag( DMS_RECORD_FLAG_DELETED ) ;
      }
   } ;
   typedef _dmsDeletedRecord dmsDeletedRecord ;
   #define DMS_DELETEDRECORD_METADATA_SZ  sizeof(dmsDeletedRecord)

   // oid + one field = 12 + 5 = 17, Algned:20
   #define DMS_MIN_DELETEDRECORD_SZ    (DMS_DELETEDRECORD_METADATA_SZ+20)
   #define DMS_MIN_RECORD_SZ           DMS_MIN_DELETEDRECORD_SZ

}

#endif //DMSRECORD_HPP_

