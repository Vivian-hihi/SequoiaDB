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

#define RTN_LOB_WRITE_PIECE_NUM 10 
#define RTN_LOB_READ_PIECE_NUM 10

namespace engine
{
   _rtnLobStream::_rtnLobStream()
   :_dpsCB( NULL ),
    _opened( FALSE ),
    _mode( 0 ),
    _lobPageSz( DMS_DO_NOT_CREATE_LOB ),
    _offset( 0 )
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
                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBSTREAM_OPEN ) ;

      rc = _prepare( fullName, oid, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to prepare to open lob[%s]"
                 " in cl[%s]", oid.str().c_str(), fullName ) ;
         goto error ;
      }

      ossMemcpy( _fullName, fullName, ossStrlen( fullName ) ) ;
      ossMemcpy( &_oid, &oid, sizeof( oid ) ) ;

      rc = _queryLobMeta( cb, _meta ) ;
      if ( SDB_OK == rc )
      {
         /// can not create a lob when it exists.
         if ( SDB_LOB_MODE_CREATEONLY & mode )
         {
            rc = SDB_FE ;
            goto error ;
         }
      }
      else if ( SDB_FNE == rc )
      {
         if ( SDB_LOB_MODE_CREATEONLY & mode )
         {
            rc = SDB_OK ;
            BOOLEAN isNew = FALSE ;
            rc = _ensureLob( cb, _meta, isNew ) ;
            if ( SDB_OK == rc )
            {
               if ( !isNew )
               {
                  rc = SDB_FE ;
                  goto error ;
               }
            }
            else
            {
               PD_LOG( PDERROR, "failed to ensure lob meta:%d", rc ) ;
               goto error ;
            }
         }
         else
         {
            goto error ;
         }
      }
      else
      {
         PD_LOG( PDERROR, "failed to get meta of lob[%s], rc:%d",
                 getOID().str().c_str(), rc ) ;
         goto error ;
      }

      _opened = TRUE ;

      rc = _getLobPageSize( _lobPageSz ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get page size of lob:%d", rc ) ;
         goto error ;
      }

      rc = _lw.init( &( getOID() ), _lobPageSz ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init stream window:%d", rc ) ;
         goto error ;
      }

      _mode = mode ;

      if ( SDB_LOB_MODE_CREATEONLY & mode )
      {
         PD_LOG( PDEVENT, "lob[%s] in [%s] is created, wait to be completed",
                 getOID().str().c_str(), fullName ) ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM_OPEN, rc ) ;
      return rc ;
   error:
      if ( _opened )
      {
         closeWithException( cb ) ;
      }
      goto done ;
   }

   INT32 _rtnLobStream::getMetaData( bson::BSONObj &meta )
   {
      INT32 rc = SDB_OK ;
      if ( !isOpened() )
      {
         rc = SDB_INVALIDARG ;
         goto done ;
      }

      if ( !_meta.isDone() )
      {
         PD_LOG( PDERROR, "not a completed lob" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( _metaObj.isEmpty() )
      {
         BSONObjBuilder builder ;
         builder.append( FIELD_NAME_LOB_SIZE, (long long)_meta._lobLen ) ;
         builder.append( FIELD_NAME_LOB_PAGE_SIZE, _lobPageSz ) ;
         _metaObj = builder.obj() ;
      }

      meta = _metaObj ;
   done:
      return rc ;
   error:
      goto done ;
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
         ossTimestamp t ;
         _dmsLobRecord piece ;
         if ( _lw.getCachedData( piece ) )
         {
            rc = _write( piece, cb ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
                       _oid.str().c_str(), rc ) ;
                goto error ;
            }
         }

         ossGetCurrentTime( t ) ;
         _meta._lobLen = _offset ;
         _meta._createTime = t.time * 1000 + t.microtm / 1000 ;

         rc = _completeLob( _meta, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to complete lob:%d", rc ) ;
            goto error ;
         }

         PD_LOG( PDEVENT, "lob [%s] is closed, len:%lld",
                 getOID().str().c_str(), _offset ) ;
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
         PD_LOG( PDERROR, "lob[%s] is closed with exception, rollback",
                 getOID().str().c_str() ) ;
         rc = _rollback( cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to rollback lob[%s], rc:%d",
                    _oid.str().c_str(), rc ) ;
            goto error ;
         }
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
      _dmsLobRecord pieces[RTN_LOB_WRITE_PIECE_NUM] ;
      UINT32 writeNum = 0 ;

      if ( !isOpened() )
      {
         PD_LOG( PDERROR, "lob[%s] is not opened yet",
                 _oid.str().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !SDB_LOB_MODE_CREATEONLY & _mode )
      {
         PD_LOG( PDERROR, "open mode[%d] does not support this operation",
                 _mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      /// re array the data and try to get a complete piece.
      rc = _lw.prepare2Write( _offset, len, buf ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to add piece to window:%d", rc ) ;
         goto error ;
      }

      /// if we update offset after write,
      /// some data will not be removed when rollback
      _offset += len ;

      while ( _lw.getNextWriteSequence( pieces[writeNum] )  )
      {
         if ( cb->isInterrupted() )
         {
            rc = SDB_INTERRUPT ;
            goto error ;
         }

         ++writeNum ;
         if ( writeNum == RTN_LOB_WRITE_PIECE_NUM )
         {
            UINT32 doneNum = 0 ;
            rc = _writev( pieces, writeNum, cb, doneNum ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
                       _oid.str().c_str(), rc ) ;
               goto error ;
            }
            writeNum = 0 ;
         }
      }

      if ( 0 != writeNum )
      {
         UINT32 doneNum = 0 ;
         rc = _writev( pieces, writeNum, cb, doneNum ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
                    _oid.str().c_str(), rc ) ;
            goto error ;
         }
         writeNum = 0 ;
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
      UINT32 needLen = 0 ;
      _dmsLobRecord pieces[RTN_LOB_READ_PIECE_NUM] ;
      UINT32 pieceNum = 0 ;
      
      SDB_ASSERT( _meta.isDone(), "lob has not been completed yet" ) ;

      if ( !isOpened() )
      {
         PD_LOG( PDERROR, "lob[%s] is not opened yet",
                 _oid.str().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( !( SDB_LOB_MODE_R & _mode ) )
      {
         PD_LOG( PDERROR, "open mode[%d] does not support this operation",
                 _mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( _meta._lobLen == _offset )
      {
         rc = SDB_EOF ;
         goto error ;
      }

      /// data may be cached.
      if ( !_pool.empty() )
      {
         if ( _pool.match( _offset ) )
         {
            rc = _readFromPool( len, context, cb, readLen ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to read data from pool:%d", rc ) ;
               goto error ;
            }

            _offset += readLen ;
            if ( _pool.empty() )
            {
               _pool.clear() ;
            }

            goto done ;
         }
         /// TODO: keep useful data in cache rather than clear all.
         else
         {
            _pool.clear() ;
         }
      }

      /// TODO: when changing page size is accepted, we should
      /// create read piece by read size rather than piece num.
      rc = _lw.prepare2Read( _meta._lobLen,
                             _offset, len,
                             needLen,
                             RTN_LOB_READ_PIECE_NUM,
                             pieces, pieceNum ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to prepare to read:%d", rc ) ;
         goto error ;      
      }

      rc = _readv( pieces, pieceNum, cb, needLen ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to write lob[%s], rc:%d",
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
      if ( !isOpened() )
      {
         PD_LOG( PDERROR, "lob[%s] is not opened yet",
                 _oid.str().c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_LOB_MODE_CREATEONLY & _mode  )
      {
         PD_LOG( PDERROR, "open mode[%d] does not support this operation",
                 _mode ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      if ( _meta._lobLen < offset )
      {
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _offset = offset ;
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
      tuple.columns.len = len <= _pool.getDataSize() ?
                          len : _pool.getDataSize() ;
      tuple.columns.offset = _offset ;
      UINT32 needLen = tuple.columns.len ;

      rc = context->appendObjs( tuple.data, sizeof( tuple.data ), 0 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to append data to context%d", rc ) ;
         goto error ;
      }

      while ( 0 < needLen )
      {
         UINT32 dataLen = 0 ;
         if ( _pool.next( needLen, &data, dataLen ) )
         {
            needLen -= dataLen ;
            readLen += dataLen ;
            rc = context->appendObjs( data, dataLen, 0 ) ;
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
         PD_LOG( PDERROR, "failed to append data to context%d", rc ) ;
         goto error ;
      }

      if ( _pool.empty() )
      {
         _pool.clear() ;
      }

   done:
      PD_TRACE_EXITRC( SDB_RTNLOBSTREAM__READFROMPOOL, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

