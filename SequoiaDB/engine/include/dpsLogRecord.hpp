/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = dpsLogRecord.hpp

   Descriptive Name = Data Protection Services Log Record

   When/how to use: this program may be used on binary and text-formatted
   versions of DPS component. This file contains declare for log record.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/05/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef DPSLOGRECORD_HPP__
#define DPSLOGRECORD_HPP__
#include "core.hpp"
#include "oss.hpp"
#include "dpsLogDef.hpp"
#include "dms.hpp"
#include "dmsRecord.hpp"
#include "pd.hpp"
namespace engine
{
   /// can not be a virtual class.
   /// since it will create virtual ptr table.
   class _dpsLogRecordHeader
   {
   public:
      _dpsLogRecordHeader()
      :_lsn( DPS_INVALID_LSN_OFFSET ),
       _preLsn( DPS_INVALID_LSN_OFFSET ),
       _length( 0 ),
       _version( DPS_INVALID_LSN_VERSION ),
       _type( LOG_TYPE_DUMMY ),
       _reserved1( 0 ),
       _reserved2( 0 )
      {

      }

      _dpsLogRecordHeader( const _dpsLogRecordHeader &header )
      :_lsn(header._lsn),
       _preLsn( header._preLsn),
       _length( header._length ),
       _version( header._version ),
       _type( header._type ),
       _reserved1( header._reserved1),
       _reserved2( header._reserved2 )
      {
      
      }

      ~_dpsLogRecordHeader(){}

      _dpsLogRecordHeader &operator=( const _dpsLogRecordHeader &header )
      {
         _lsn = header._lsn ;
         _preLsn = header._preLsn ;
         _length = header._length ;
         _version = header._version ;
         _type = header._type ;
         _reserved1 = header._reserved1 ;
         _reserved2 = header._reserved2 ;
         return *this ;
      }

      void clear()
      {
         _lsn = DPS_INVALID_LSN_OFFSET ;
         _preLsn = DPS_INVALID_LSN_OFFSET ;
         _length = 0 ;
         _version = DPS_INVALID_LSN_VERSION ;
         _type = LOG_TYPE_DUMMY ;
         _reserved1 = 0 ;
         _reserved2 = 0 ;
      }

   public:
      // 0x00 - 0x07
      DPS_LSN_OFFSET _lsn;
      // 0x08 - 0x0F
      DPS_LSN_OFFSET _preLsn;
      // 0x10 - 0x13
      UINT32 _length;
      // 0x14 - 0x17
      DPS_LSN_VER _version ;
      // 0x18 - 0x19
      UINT16 _type;
      // 0x1A - 0x1B
      UINT16 _reserved1 ;
      // 0x1C - 0x1F
      UINT32 _reserved2 ;
   } ;
   typedef class _dpsLogRecordHeader dpsLogRecordHeader ;

#pragma pack(1)
   class _dpsRecordEle
   {
   public:
      _dpsRecordEle()
      :tag(DPS_INVALID_TAG),
       len(0)
      {}
   public:
      DPS_TAG tag ;
      UINT32 len ;
   } ;
   typedef class _dpsRecordEle dpsRecordEle ;
#pragma pack()

   /// record ::= header body
   /// body ::= +(item)
   /// item ::= tag len value
   /// tag ::= UINT8
   /// len ::= UINT32
   /// value ::= CHAR *
   class _dpsLogRecord : public SDBObject
   {
   public:
      class iterator : public SDBObject
      {
      public:
         iterator( const _dpsLogRecord *record )
         :_current(-1),
          _record( record )
         {
            SDB_ASSERT( NULL != _record, "impossible" )
         }

         iterator( const iterator &itr )
         :_current( itr._current ),
          _record( itr._record )
         {

         }

         iterator()
         :_current(-1),
          _record(NULL)
         {

         }

         const iterator &operator=( const iterator &itr )
         {
            _current = itr._current ;
            _record = itr._record ;
            return *this ;
         }

         virtual ~iterator()
         {
            _current = 0 ;
            _record = NULL ;
         }
    public:
         inline BOOLEAN next()
         {
            if ( DPS_MERGE_BLOCK_MAX_DATA == ++_current )
            {
               return FALSE ;
            }
            else
            {
               return NULL != (_record->_data)[_current] ;
            }
         }

         inline UINT32 len() const
         {
            return (_record->_dataHeader)[_current].len ;
         }

         inline UINT8 tag() const
         {
            return (_record->_dataHeader)[_current].tag ;
         }

         inline const _dpsRecordEle &dataMeta() const
         {
            return (_record->_dataHeader)[_current] ;
         }

         inline const CHAR *value() const
         {
            return (_record->_data)[_current] ;
         }

         inline BOOLEAN valid() const
         {
            return ( 0 <= _current && _current < DPS_MERGE_BLOCK_MAX_DATA ) &&
                   ( NULL != _record ) ;
         }

      private:
         INT32 _current ;
         const _dpsLogRecord *_record ;

      friend class _dpsLogRecord ;
   } ;
   public :
      _dpsLogRecord () ;
      _dpsLogRecord( const _dpsLogRecord &record ) ;
      virtual ~_dpsLogRecord () ;
   public:
      UINT32 dump ( CHAR *outBuf, UINT32 outSize, UINT32 options ) const ;

      INT32 load ( const CHAR *pData ) ;

      iterator find( DPS_TAG tag ) const ;

      INT32 push( DPS_TAG tag, UINT32 len, const CHAR *value ) ;

      UINT32 alignedLen() const ;

      void clear() ;

      inline dpsLogRecordHeader &head ()
      {
         return _head ;
      }

      inline const dpsLogRecordHeader &head() const
      {
         return _head ;
      }

      _dpsLogRecord &operator=(const _dpsLogRecord &) ;

   private:
      dpsLogRecordHeader _head ;
      const CHAR *_data[DPS_MERGE_BLOCK_MAX_DATA] ;
      _dpsRecordEle _dataHeader[DPS_MERGE_BLOCK_MAX_DATA] ;
      UINT32 _write ;
   } ;
   typedef class _dpsLogRecord dpsLogRecord ;

const UINT32 DPS_RECORD_MAX_LEN = DMS_COLLECTION_SPACE_NAME_SZ +
                                  DMS_COLLECTION_NAME_SZ + 2 +
                                  sizeof( _dpsLogRecordHeader ) +
                                  DMS_RECORD_USER_MAX_SZ * 2 ;
}


#endif
