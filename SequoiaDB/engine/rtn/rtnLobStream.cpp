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

   Source File Name = rtnLobStream.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnLobStream.hpp"
#include "pmdEDU.hpp"
#include "msgDef.h"
#include "pdTrace.hpp"
#include "rtnTrace.hpp"
#include "rtnContext.hpp"

using namespace bson ;

#define ALLOC_MEM( needLen, len, buf, rc ) \
        do\
        {\
           if ( needLen <= len )\
           {\
           }\
           else if ( NULL == buf )\
           {\
              buf = ( CHAR * )SDB_OSS_MALLOC( needLen ) ;\
              if ( NULL == buf )\
              {\
                 rc = SDB_OOM ;\
              }\
              len = needLen ;\
           }\
           else\
           {\
              SDB_OSS_FREE( buf ) ;\
              buf = NULL ;\
              len = 0 ;\
              buf = ( CHAR * )SDB_OSS_MALLOC( needLen ) ;\
              if ( NULL == buf )\
              {\
                 rc = SDB_OOM ;\
              }\
              len = needLen ;\
           }\
        } while ( FALSE )

namespace engine
{
   _rtnLobStream::_rtnLobStream()
   :_dpsCB( NULL ),
    _opened( FALSE ),
    _mode( 0 ),
    _flags( 0 ),
    _lobPageSz( DMS_DO_NOT_CREATE_LOB ),
    _logarithmic( 0 ),
    _offset( 0 ),
    _hasPiecesInfo( FALSE )
   {
      ossMemset( _fullName, 0, DMS_COLLECTION_SPACE_NAME_SZ +
                               DMS_COLLECTION_NAME_SZ + 2 ) ;
   }

   _rtnLobStream::~_rtnLobStream()
   {

   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM_OPEN, "_rtnLobStream::open" )
   INT32 _rtnLobStream::open( const CHAR *fullName,
                              const bson::OID &oid,
                              INT32 mode,
                              INT32 flags,
                              _rtnContextBase *context,
                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_OPEN ) ;

      ossMemcpy( _fullName, fullName, ossStrlen( fullName ) ) ;
      ossMemcpy( &_oid, &oid, sizeof( oid ) ) ;
      _mode = mode ;
      _flags = flags ;

      if ( SDB_LOB_MODE_CREATEONLY == mode )
      {
         _meta._createTime = ossGetCurrentMilliseconds() ;
         _meta._modificationTime = _meta._createTime ;
      }

      rc = _prepare( fullName, oid, mode, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to prepare to open lob[%s]"
                 " in collection[%s], rc:%d",
                 oid.str().c_str(), fullName, rc ) ;
         goto error ;
      }

      if ( SDB_LOB_MODE_READ == mode )
      {
         rc = _open4Read( cb ) ;
         /// AUDIT
         PD_AUDIT_OP_WITHNAME( AUDIT_DQL, "LOB READ", AUDIT_OBJ_CL,
                               getFullName(), rc,
                               "OID:%s, Length:%llu, CreateTime:%llu, ModificationTime:%llu",
                               getOID().toString().c_str(),
                               _meta._lobLen, _meta._createTime, _meta._modificationTime ) ;
      }
      else if ( SDB_LOB_MODE_CREATEONLY == mode )
      {
         rc = _open4Create( cb ) ;
      }
      else if ( SDB_LOB_MODE_REMOVE == mode )
      {
         rc = _open4Remove( cb ) ;
      }
      else
      {
         PD_LOG( PDERROR, "unknown open mode:%d", mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to open lob[%s], rc:%d",
                 oid.str().c_str(), rc ) ;
         goto error ;
      }

      /// get page size, must call before _meta2Obj
      rc = _getLobPageSize( _lobPageSz ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to get page size of lob, rc:%d", rc ) ;
         goto error ;
      }

      if ( !ossIsPowerOf2( _lobPageSz, &_logarithmic ) )
      {
         PD_LOG( PDERROR, "Invalid page size:%d, it should be a power of 2",
                 _lobPageSz ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      /// if has context, need copy metaObj and data to context
      if ( context )
      {
         _metaObj = _meta2Obj( _meta ) ;
         rc = context->append( _metaObj ) ;
         if ( rc )
         {
            PD_LOG( PDERROR, "Failed to append meta data, rc:%d", rc ) ;
            goto error ;
         }
         /// add the data
         if ( _pool.getLastDataSize() > 0 &&
              ( _flags & FLG_LOBOPEN_WITH_RETURNDATA ) )
         {
            UINT32 readLen = 0 ;
            UINT32 poolSize = _pool.getLastDataSize() ;
            rc = _readFromPool( poolSize, context, cb, readLen ) ;
            if ( rc )
            {
               PD_LOG( PDERROR, "Failed to read from pool, rc:%d", rc ) ;
               goto error ;
            }
            _offset += readLen ;
         }
      }

      rc = _lw.init( _lobPageSz,
                     _meta._version >= DMS_LOB_META_MERGE_DATA_VERSION ?
                     TRUE : FALSE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to init stream window, rc:%d", rc ) ;
         goto error ;
      }

      _opened = TRUE ;
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_OPEN, rc ) ;
      return rc ;
   error:
      closeWithException( cb ) ;
      goto done ;
   }

   BSONObj _rtnLobStream::_meta2Obj( const _dmsLobMeta &meta ) const
   {
      BSONObjBuilder builder ;
      /// we can get nothing when mode is create.
      builder.append( FIELD_NAME_LOB_SIZE, meta._lobLen ) ;
      builder.append( FIELD_NAME_LOB_PAGE_SIZE, _lobPageSz ) ;
      builder.append( FIELD_NAME_VERSION, (INT32)meta._version ) ;
      builder.append( FIELD_NAME_LOB_CREATTIME, (INT64)meta._createTime ) ;
      builder.append( FIELD_NAME_LOB_MODIFICATION_TIME, (INT64)meta._modificationTime ) ;
      return builder.obj() ;
   }

   UINT32 _rtnLobStream::_getSequence( INT64 offset ) const
   {
      SDB_ASSERT( isOpened(), "not opened" ) ;

      return RTN_LOB_GET_SEQUENCE( offset,
                                   _meta._version >= DMS_LOB_META_MERGE_DATA_VERSION,
                                   _logarithmic ) ;
   }

   INT32 _rtnLobStream::getMetaData( bson::BSONObj &meta )
   {
      INT32 rc = SDB_OK ;
      if ( !isOpened() )
      {
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      if ( _metaObj.isEmpty() )
      {
         _metaObj = _meta2Obj( _meta ) ;
      }

      meta = _metaObj ;
   done:
      return rc ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM_CLOSE, "_rtnLobStream::close" )
   INT32 _rtnLobStream::close( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_CLOSE ) ;
      if ( !isOpened() )
      {
         goto done ;
      }

      if ( SDB_LOB_MODE_CREATEONLY & _mode )
      {
         rc = _close4Create( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close for create:%d", rc ) ;
            goto error ;
         }
      }
      else if ( SDB_LOB_MODE_REMOVE & _mode )
      {
         RTN_LOB_TUPLES tuples ;
         _rtnLobTuple tuple( 0, DMS_LOB_META_SEQUENCE, 0, NULL ) ;
         tuples.push_back( tuple ) ;
         if ( _meta._flag & DMS_LOB_META_FLAG_PIECESINFO_PAGE )
         {
            _rtnLobTuple piecesInfoTuple( 0, DMS_LOB_META_SEQUENCE, 0, NULL ) ;
            tuples.push_back( piecesInfoTuple ) ;
         }

         rc = _removev( tuples, cb ) ;
         PD_AUDIT_OP_WITHNAME( AUDIT_DML, "LOB REMOVE", AUDIT_OBJ_CL,
                               getFullName(), rc, "OID:%s, Meta:%s",
                               getOID().toString().c_str(),
                               _metaObj.toString().c_str() ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to remove meta data of lob:%d", rc ) ;
            goto error ;
         }
         PD_LOG( PDDEBUG, "lob [%s] is removed",
                 getOID().str().c_str() ) ;
      }

      rc = _close( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to close lob:%d", rc ) ;
         goto error ;
      }
      _opened = FALSE ;
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_CLOSE, rc ) ;
      return rc ;
   error:
      closeWithException( cb ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM_CLOSEWITHEXCEPTION, "_rtnLobStream::closeWithException" )
   INT32 _rtnLobStream::closeWithException( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_CLOSEWITHEXCEPTION ) ;
      if ( !isOpened() )
      {
         goto done ;
      }

      if ( SDB_LOB_MODE_CREATEONLY & _mode ) 
      {
         PD_LOG( PDERROR, "Lob[%s] is closed with exception, rollback",
                 getOID().str().c_str() ) ;
         rc = _rollback( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rollback lob[%s], rc:%d",
                    _oid.str().c_str(), rc ) ;
            goto error ;
         }
      }
      else
      {
         PD_LOG( PDWARNING, "Lob[%s] is closed with exception, mode:0x%08x",
                 getOID().str().c_str(), _mode ) ;
      }

      _opened = FALSE ;
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_CLOSEWITHEXCEPTION, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM_WRITE, "_rtnLobStream::write" )
   INT32 _rtnLobStream::write ( UINT32 len,
                                const CHAR *buf,
                                _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_WRITE ) ;
      RTN_LOB_TUPLES tuples ;

      if ( !isOpened() )
      {
         PD_LOG( PDERROR, "lob[%s] is not opened yet",
                 _oid.str().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !( SDB_LOB_MODE_CREATEONLY & _mode ) )
      {
         PD_LOG( PDERROR, "open mode[%d] does not support this operation",
                 _mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      // re-array the data and try to get a complete piece.
      rc = _lw.prepare4Write( _offset, len, buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to add piece to window, rc:%d", rc ) ;
         goto error ;
      }

      // if we update offset after write,
      // some data will not be removed when rollback
      _offset += len ;
      // update lobLen immediately,
      // for _offset can be set to front position by seek 
      _meta._lobLen = OSS_MAX( _meta._lobLen, _offset ) ;

      while ( _lw.getNextWriteSequences( tuples )  )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_INTERRUPT ;
            goto error ;
         }

         rc = _writeOrUpdate( tuples, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
                    _oid.str().c_str(), rc ) ;
            goto error ;
         }

         tuples.clear() ;
      }

      _lw.cacheLastDataOrClearCache() ;

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_WRITE, rc ) ;
      return rc ;
   error:
      closeWithException( cb ) ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM_READ, "_rtnLobStream::read" )
   INT32 _rtnLobStream::read( UINT32 len,
                              _rtnContextBase *context,
                              _pmdEDUCB *cb,
                              UINT32 &read )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_READ ) ;
      UINT32 readLen = 0 ;
      RTN_LOB_TUPLES tuples ;

      SDB_ASSERT( _meta.isDone(), "lob has not been completed yet" ) ;

      if ( !isOpened() )
      {
         PD_LOG( PDERROR, "lob[%s] is not opened yet",
                 _oid.str().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !( SDB_LOB_MODE_READ & _mode ) )
      {
         PD_LOG( PDERROR, "open mode[%d] does not support this operation",
                 _mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( 0 == len )
      {
         goto done ;
         read = 0 ;
      }

      if ( _meta._lobLen <= _offset )
      {
         rc = SDB_EOF ;
         goto error ;
      }
      else if ( _offset + len > _meta._lobLen )
      {
         len = _meta._lobLen - _offset ;
      }

      /// data may be cached.
      if ( _pool.match( _offset ) )
      {
         rc = _readFromPool( len, context, cb, readLen ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to read data from pool:%d", rc ) ;
            goto error ;
         }

         _offset += readLen ;
         goto done ;
      }

      /// clear cache when we can not get data from it.
      _pool.clear() ;

      /// reset the read len of a suitable value
      rc = _lw.prepare4Read( _meta._lobLen,
                             _offset, len,
                             tuples ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to prepare to read:%d", rc ) ;
         goto error ;      
      }

      rc = _readv( tuples, cb, _hasPiecesInfo ? &_lobPieces : NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read lob[%s], rc:%d",
                    _oid.str().c_str(), rc ) ;
         goto error ;
      }

      rc = _readFromPool( len, context, cb, readLen ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read data from pool:%d", rc ) ;
         goto error ;
      }

      _offset += len ;
      read = readLen ;
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_READ, rc ) ;
      return rc ;
   error:
      closeWithException( cb ) ;
      goto done ;
   }

   INT32 _rtnLobStream::seek( SINT64 offset, _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      SDB_ASSERT( offset >= 0, "invalid offset" ) ;

      if ( !isOpened() )
      {
         PD_LOG( PDERROR, "lob[%s] is not opened yet",
                 _oid.str().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_LOB_MODE_READ != _mode && SDB_LOB_MODE_CREATEONLY != _mode )
      {
         PD_LOG( PDERROR, "open mode[%d] does not support this operation",
                 _mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( offset >= _meta._lobLen && SDB_LOB_MODE_READ == _mode )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( SDB_LOB_MODE_CREATEONLY == _mode )
      {
         if ( !_lw.continuous( offset ) )
         {
            _rtnLobTuple tuple ;
            // write last data
            if ( _lw.getCachedData( tuple ) )
            {
               rc = _writeOrUpdate( tuple, cb ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
                          _oid.str().c_str(), rc ) ;
                   goto error ;
               }
            }

            if ( !_hasPiecesInfo )
            {
               UINT32 piece = _getSequence( _offset ) ;
               _lobPieces.addPieces( _rtnLobPieces(0, piece) ) ;
               _hasPiecesInfo = TRUE ;
            }
         }
      }

      _offset = offset ;

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM_TRUNCATE, ""_rtnLobStream::truncate" )
   INT32 _rtnLobStream::truncate( SINT64 len,
                                  _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_TRUNCATE ) ;
      SDB_ASSERT( SDB_LOB_MODE_REMOVE == _mode && 0 == len,
                  "do not support other params now" ) ;

      RTN_LOB_TUPLES tuples ;
      UINT32 pieceNum = 0 ;
      UINT32 oneLoopNum = 0 ;

      RTN_LOB_GET_SEQUENCE_NUM( _meta._lobLen, _lobPageSz,
                                _meta._version >= DMS_LOB_META_MERGE_DATA_VERSION,
                                pieceNum ) ;
      while ( 1 < pieceNum-- )
      {
         tuples.push_back( _rtnLobTuple( 0, pieceNum, 0, NULL ) ) ;
         ++oneLoopNum ;

         if ( 1000 == oneLoopNum )
         {
            rc = _removev( tuples, cb ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to truncate lob:%d", rc ) ;
               goto error ;
            }

            oneLoopNum = 0 ;
            tuples.clear() ;
         }
      }

      if ( !tuples.empty() )
      {
         rc = _removev( tuples, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to truncate lob:%d", rc ) ;
            goto error ;
         }
         tuples.clear() ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_TRUNCATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnLobStream::_writeOrUpdate( const _rtnLobTuple &tuple,
                                        _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      if ( !_hasPiecesInfo )
      {
         rc = _write( tuple, cb ) ;
      }
      else
      {
         if ( !_lobPieces.hasPiece( tuple.tuple.columns.sequence ) )
         {
            rc = _write( tuple, cb ) ;
            if ( SDB_OK == rc )
            {
               rc = _lobPieces.addPiece( tuple.tuple.columns.sequence ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "Failed to add piece, rc=%d", rc ) ;
                  goto error ;
               }

               if ( _lobPieces.requiredMem() > _lobPageSz )
               {
                  rc = SDB_LOB_PIECESINFO_OVERFLOW ;
                  PD_LOG( PDERROR, "LOB pieces info require memory more than one lob page, "\
                                   "section num=%d, piecesInfo=%s",
                                   _lobPieces.sectionNum(), _lobPieces.toString().c_str() ) ;
                  goto error ;
               }
            }
         }
         else
         {
            rc = _update( tuple, cb ) ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnLobStream::_writeOrUpdate( RTN_LOB_TUPLES &tuples,
                                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;

      if ( !_hasPiecesInfo )
      {
         rc = _writev( tuples, cb ) ;
      }
      else
      {
         RTN_LOB_TUPLES updateTuples ;

         for ( RTN_LOB_TUPLES::iterator iter = tuples.begin() ;
               iter != tuples.end() ; )
         {
            _rtnLobTuple& tuple = *iter ;
            if ( _lobPieces.hasPiece( tuple.tuple.columns.sequence ) )
            {
               updateTuples.push_back( tuple ) ;
               iter = tuples.erase( iter ) ;
            }
            else
            {
               ++iter ;
            }
         }

         if ( !tuples.empty() )
         {
            rc = _writev( tuples, cb ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }

            for ( RTN_LOB_TUPLES::const_iterator iter = tuples.begin() ;
                  iter != tuples.end() ; iter++ )
            {
               const _rtnLobTuple& tuple = *iter ;
               rc = _lobPieces.addPiece( tuple.tuple.columns.sequence ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "Failed to add piece, rc=%d", rc ) ;
                  goto error ;
               }

               if ( _lobPieces.requiredMem() > _lobPageSz )
               {
                  rc = SDB_LOB_PIECESINFO_OVERFLOW ;
                  PD_LOG( PDERROR, "LOB pieces info require memory more than one lob page, "\
                                   "section num=%d, piecesInfo=%s",
                                   _lobPieces.sectionNum(), _lobPieces.toString().c_str() ) ;
                  goto error ;
               }
            }
         }

         if ( !updateTuples.empty() )
         {
            rc = _updatev( updateTuples, cb ) ;
            if ( SDB_OK != rc )
            {
               goto error ;
            }
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM__READFROMPOOL, "_rtnLobStream::_readFromPool" )
   INT32 _rtnLobStream::_readFromPool( UINT32 len,
                                       _rtnContextBase *context,
                                       _pmdEDUCB *cb,
                                       UINT32 &readLen )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM__READFROMPOOL ) ;
      const CHAR *data = NULL ;
      _MsgLobTuple tuple ;
      tuple.columns.len = len <= _pool.getLastDataSize() ?
                          len : _pool.getLastDataSize() ;
      tuple.columns.offset = _offset ;
      tuple.columns.sequence = 0 ; /// it is useless column now.
      UINT32 needLen = tuple.columns.len ;

      SDB_ASSERT( _pool.match( _offset ), "impossible" ) ;

      rc = context->appendObjs( tuple.data, sizeof( tuple.data ), 0 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to append data to context%d", rc ) ;
         goto error ;
      }

      while ( 0 < needLen )
      {
         UINT32 dataLen = 0 ;
         if( _pool.next( needLen, &data, dataLen ) )
         {
            needLen -= dataLen ;
            readLen += dataLen ;
            rc = context->appendObjs( data, dataLen, 0, FALSE ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to append data to context%d", rc ) ;
               goto error ;
            }
         }
         else
         {
            break ;
         }
      }

      SDB_ASSERT( readLen == tuple.columns.len, "impossible" ) ;
      rc = context->appendObjs( NULL, 0, 1 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to append data to context, rc:%d", rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM__READFROMPOOL, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM__OPEN4READ, "_rtnLobStream::_open4Read" )
   INT32 _rtnLobStream::_open4Read( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM__OPEN4READ ) ;

      rc = _queryLobMeta( cb, _meta, &_lobPieces ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to open lob[%s] in collection[%s], rc:%d",
                 _oid.str().c_str(), _fullName, rc ) ;
         goto error ;
      }

      if ( _lobPieces.sectionNum() > 0 )
      {
         _hasPiecesInfo = TRUE ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM__OPEN4READ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM__OPEN4CREATE, "_rtnLobStream::_open4Create" )
   INT32 _rtnLobStream::_open4Create( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM__OPEN4CREATE ) ;
      BOOLEAN isNew = TRUE ;
      rc = _ensureLob( cb, _meta, isNew ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to open lob[%s] in collection[%s], rc:%d",
                 _oid.str().c_str(), _fullName, rc ) ;
         goto error ;
      }

      if ( !isNew )
      {
         PD_LOG( PDERROR, "Lob[%s] exists in collection[%s]",
                 _oid.str().c_str(), _fullName ) ;
         rc = SDB_FE ;
         goto error ;
      }

      PD_LOG( PDDEBUG, "Lob[%s] in [%s] is created, wait to be completed",
              getOID().str().c_str(), _fullName ) ;

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM__OPEN4CREATE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBSTREAM__OPEN4REMOVE, "_rtnLobStream::_open4Remove" )
   INT32 _rtnLobStream::_open4Remove( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM__OPEN4REMOVE ) ;

      rc = _queryAndInvalidateMetaData( cb, _meta ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open lob[%s] in collection[%s], rc:%d",
                 _oid.str().c_str(), _fullName, rc ) ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM__OPEN4REMOVE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnLobStream::_close4Create( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      CHAR* buf = NULL ;
      INT32 piecesInfoSize = 0 ;
      _rtnLobTuple tuple ;

      SDB_ASSERT( SDB_LOB_MODE_CREATEONLY & _mode, "incorrect mode" ) ;

      // write last data
      if ( _lw.getCachedData( tuple ) )
      {
         rc = _writeOrUpdate( tuple, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
                    _oid.str().c_str(), rc ) ;
             goto error ;
         }
      }

      if ( _hasPiecesInfo )
      {
         piecesInfoSize = _lobPieces.requiredMem() ;
         if( piecesInfoSize > _lobPageSz )
         {
            PD_LOG( PDERROR, "LOB pieces info require memory more than one lob page, "\
                             "section num=%d, piecesInfo=%s",
                             _lobPieces.sectionNum(), _lobPieces.toString().c_str() ) ;
            rc = SDB_LOB_PIECESINFO_OVERFLOW ;
            goto error ;
         }

         if ( piecesInfoSize > 0 && _lobPieces.sectionNum() == 1 )
         {
            UINT32 last = _getSequence( _meta._lobLen ) ;
            _rtnLobPieces pieces = _lobPieces.getSection( 0 ) ;
            if ( 0 == pieces.first && last == pieces.last )
            {
               // no skipped piece, so no need to save pieces info
               piecesInfoSize= 0 ;
            }
         }
      }

      // write lob pieces info page
      if ( piecesInfoSize > DMS_LOB_META_PIECESINFO_LEN )
      {         
         buf = (CHAR*) SDB_OSS_MALLOC( piecesInfoSize ) ;
         if ( NULL == buf )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "failed to alloc memory for lob pieces info, rc=%d", rc ) ;
            goto error ;
         }

         rc = _lobPieces.saveTo( buf, piecesInfoSize ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to save lob pieces info, rc=%d", rc ) ;
            goto error ;
         }

         tuple.tuple.columns.len = piecesInfoSize ;
         tuple.tuple.columns.sequence = DMS_LOB_PIECESINFO_SEQUENCE ;
         tuple.tuple.columns.offset = 0 ;
         tuple.data = ( const CHAR* )buf ;

         rc = _write( tuple, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write lob pieces, rc=%d", rc ) ;
            goto error ;
         }

         _meta._piecesInfoNum = _lobPieces.sectionNum() ;
         _meta._flag |= DMS_LOB_META_FLAG_PIECESINFO_PAGE ;
      }

      // write meta data
      // _meta._lobLen is already updated
      _meta._modificationTime = ossGetCurrentMilliseconds() ;
      _meta._status = DMS_LOB_COMPLETE ;
      if ( _lw.getMetaPageData( tuple ) )
      {
         if ( piecesInfoSize > 0 &&
              piecesInfoSize <= DMS_LOB_META_PIECESINFO_LEN )
         {
            CHAR* piecesInfoBuf = (CHAR*)tuple.data + DMS_LOB_META_LENGTH 
                                  - piecesInfoSize ;

            rc = _lobPieces.saveTo( piecesInfoBuf, piecesInfoSize ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to save lob pieces info, rc=%d", rc ) ;
               goto error ;
            }

            _meta._piecesInfoNum = _lobPieces.sectionNum() ;
            _meta._flag |= DMS_LOB_META_FLAG_PIECESINFO_INSIDE ;
         }

         ossMemcpy( (CHAR*)tuple.data, (const CHAR*)&_meta,
                     sizeof( _meta ) ) ;
      }
      else if ( piecesInfoSize > 0 &&
                piecesInfoSize <= DMS_LOB_META_PIECESINFO_LEN )
      {
         SDB_ASSERT( NULL == buf, "impossible" ) ;

         buf = (CHAR*) SDB_OSS_MALLOC( DMS_LOB_META_LENGTH ) ;
         if ( NULL == buf )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "failed to alloc memory for lob pieces info, rc=%d", rc ) ;
            goto error ;
         }

         _meta._piecesInfoNum = _lobPieces.sectionNum() ;
         _meta._flag |= DMS_LOB_META_FLAG_PIECESINFO_INSIDE ;

         ossMemcpy( buf, (const CHAR*)&_meta, sizeof( _meta ) ) ;

         CHAR* piecesInfoBuf = buf + DMS_LOB_META_LENGTH 
                               - piecesInfoSize ;

         rc = _lobPieces.saveTo( piecesInfoBuf, piecesInfoSize ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to save lob pieces info, rc=%d", rc ) ;
            goto error ;
         }

         tuple.tuple.columns.len = DMS_LOB_META_LENGTH ;
         tuple.tuple.columns.sequence = DMS_LOB_META_SEQUENCE ;
         tuple.tuple.columns.offset = 0 ;
         tuple.data = ( const CHAR* )buf ;
      }
      else
      {
         tuple.tuple.columns.len = sizeof( _meta ) ;
         tuple.tuple.columns.sequence = DMS_LOB_META_SEQUENCE ;
         tuple.tuple.columns.offset = 0 ;
         tuple.data = ( const CHAR* )&_meta ;
      }

      rc = _completeLob( tuple, cb ) ;
      PD_AUDIT_OP_WITHNAME( AUDIT_DML, "LOB CREATE", AUDIT_OBJ_CL,
                            getFullName(), rc, "OID:%s, Length:%llu",
                            getOID().toString().c_str(),
                            _meta._lobLen ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to complete lob:%d", rc ) ;
         goto error ;
      }

      PD_LOG( PDDEBUG, "lob [%s] is closed, len:%lld",
              getOID().str().c_str(), _offset ) ;

   done:
      SAFE_OSS_FREE( buf ) ;
      return rc ;
   error:
      goto done ;
   }
}

