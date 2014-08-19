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

   Source File Name = rtnLobWindow.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnLobWindow.hpp"
#include "rtnTrace.hpp"
#include "pdTrace.hpp"

namespace engine
{
   _rtnLobWindow::_rtnLobWindow()
   :_oid( NULL ),
    _pageSize( DMS_DO_NOT_CREATE_LOB ),
    _logarithmic( 0 ),
    _curOffset( 0 ),
    _pool( NULL ),
    _cachedSz( 0 ),
    _analysisCache( FALSE )
   {

   }

   _rtnLobWindow::~_rtnLobWindow()
   {
      if ( NULL != _pool )
      {
         SDB_OSS_FREE( _pool ) ;
         _pool = NULL ; 
      }
   }

   INT32 _rtnLobWindow::init( const bson::OID *oid, INT32 pageSize )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( DMS_DO_NOT_CREATE_LOB < pageSize &&
                  NULL != oid,
                  "invalid arguments" ) ;
      SDB_ASSERT( _writeData.empty(), "impossible" ) ;

      if ( !ossIsPowerOf2( pageSize, &_logarithmic ) )
      {
         PD_LOG( PDERROR, "invalid page size:%d, it should be a power of 2",
                 pageSize ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      _pool = ( CHAR * )SDB_OSS_MALLOC( pageSize ) ;
      if ( NULL == _pool )
      {
         PD_LOG( PDERROR, "failed to allocate mem." ) ;
         rc = SDB_OOM ;
         goto error ;
      }

      _pageSize = pageSize ;
      _oid = oid ;
   done:
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBWINDOW_ADDOUTPUTDATA, "_rtnLobWindow::addOutputData" )
   INT32 _rtnLobWindow::prepare2Write( SINT64 offset, UINT32 len, const CHAR *data )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBWINDOW_ADDOUTPUTDATA ) ;
      SDB_ASSERT( 0 <= offset && NULL != data, "invalid arguments" ) ;
      SDB_ASSERT( _writeData.empty(), "the last write has not been done" ) ;
      
      /// TOOD: seek write ?
      if ( offset != _curOffset )
      {
         PD_LOG( PDERROR, "invalid offset:%lld, current offset:%lld"
                 ", we do not support seek write yet",
                 offset, _curOffset ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }

      /// never cached data
      if ( 0 == _cachedSz )
      {
         _writeData.offset = offset ;
         _writeData.len = len ;
         _writeData.data = data ;
         _analysisCache = FALSE ;
      }
      else
      {
         /// join cached data and write data.
         SDB_ASSERT( _cachedSz < _pageSize, "impossible" ) ;
         INT32 mvSize = _pageSize - _cachedSz ;
         mvSize = ( UINT32 )mvSize <= len ? mvSize : len ;
         ossMemcpy( _pool + _cachedSz, data, mvSize ) ;
         _cachedSz += mvSize ;
         if ( 0 != len - mvSize )
         {
            _writeData.offset = offset + mvSize ;
            _writeData.len = len - mvSize ;
            _writeData.data = data + mvSize ;
         }

         _analysisCache = TRUE ;
      }
      _curOffset += len ;
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBWINDOW_ADDOUTPUTDATA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBWINDOW_GETNEXTWRITESEQUENCE, "_rtnLobWindow::getNextWriteSequence" )
   BOOLEAN _rtnLobWindow::getNextWriteSequence( _dmsLobRecord &piece )
   {
      PD_TRACE_ENTRY( SDB_RTNLOBWINDOW_GETNEXTWRITESEQUENCE ) ;
      BOOLEAN hasNext = FALSE ;

      if ( !_analysisCache )
      {
         if ( (UINT32)_pageSize < _writeData.len )
         {
            piece.set( _oid, RTN_LOB_GET_SEQUENCE(( _writeData.offset ),_logarithmic),
                       0, _pageSize, _writeData.data ) ;
            _writeData.len -= _pageSize ;
            _writeData.offset += _pageSize ;
            _writeData.data += _pageSize ;
            hasNext = TRUE ;
         }
         else if ( (UINT32)_pageSize == _writeData.len )
         {
            piece.set( _oid, RTN_LOB_GET_SEQUENCE(( _writeData.offset ),_logarithmic),
                       0, _pageSize, _writeData.data ) ;
            hasNext = TRUE ;
            _writeData.clear() ;
         }
         else
         {
            /// cache data
            goto done ;
         }
      }
      else if ( _pageSize == _cachedSz )
      {
         /// we should find the exact offset.
         SINT64 offset = _writeData.empty() ? _curOffset : _writeData.offset ;
         piece.set( _oid, RTN_LOB_GET_SEQUENCE(( offset - _cachedSz ),_logarithmic),
                       0, _cachedSz, _pool ) ;
         hasNext = TRUE ;
         _analysisCache = FALSE ;
      }
      else
      {
         SDB_ASSERT( _writeData.empty(), "should be joined before" ) ;
      }
   done:
      PD_TRACE_EXIT( SDB_RTNLOBWINDOW_GETNEXTWRITESEQUENCE ) ;
      return hasNext ;
   }

   void _rtnLobWindow::cacheLastDataOrClearCache()
   {
      if ( _pageSize == _cachedSz )
      {
         _cachedSz = 0 ;
      }

      if ( !_writeData.empty() )
      {
         SDB_ASSERT( _writeData.len < ( UINT32 )_pageSize, "impossible" ) ;
         SDB_ASSERT( 0 == _cachedSz || _cachedSz == _pageSize, "impossible" ) ;
         ossMemcpy( _pool, _writeData.data, _writeData.len ) ;
         _cachedSz += _writeData.len ;
         _writeData.clear() ;
      }

      _analysisCache = FALSE ;
      return ;
   }

   BOOLEAN _rtnLobWindow::getCachedData( _dmsLobRecord &piece )
   {
      BOOLEAN hasNext = FALSE ;
      if ( 0 == _cachedSz )
      {
         goto done ;
      }

      piece.set( _oid,
                 RTN_LOB_GET_SEQUENCE( _curOffset - _cachedSz, _logarithmic ),
                 0, _cachedSz, _pool ) ;
      hasNext = TRUE ;
      _cachedSz = 0 ;
      _analysisCache = FALSE ; 
   done:
      return hasNext ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNLOBWINDOW_PREPARE2READ, "_rtnLobWindow::_rtnLobWindow::prepare2Read" )
   INT32 _rtnLobWindow::prepare2Read( SINT64 lobLen,
                                      SINT64 offset,
                                      UINT32 len,
                                      UINT32 &readLen,
                                      UINT32 maxPieceNum,
                                      _dmsLobRecord *pieces,
                                      UINT32 &pieceNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNLOBWINDOW_PREPARE2READ ) ;
      SDB_ASSERT( offset < lobLen, "impossible" ) ;
      UINT32 i = 0 ;
      while ( readLen < len && offset < lobLen && i < maxPieceNum )
      {
         UINT32 offstInPiece = RTN_LOB_GET_OFFSET_IN_SEQUENCE( offset, _pageSize ) ;
//         UINT32 lenInPiece =  ( _pageSize - offstInPiece ) < len ?
//                              ( _pageSize - offstInPiece ) : len ;
         UINT32 lenInPiece = _pageSize - offstInPiece ;
         lenInPiece = lenInPiece <= ( lobLen - offset ) ?
                      lenInPiece : ( lobLen - offset ) ;
         UINT32 sequence = RTN_LOB_GET_SEQUENCE( offset, _logarithmic ) ;

         offset += lenInPiece ;
         readLen += lenInPiece ;

         pieces[i++].set( _oid, sequence, offstInPiece,
                          lenInPiece, NULL ) ;
      }
      
      pieceNum = i ;
   done:
      PD_TRACE_EXITRC( SDB_RTNLOBWINDOW_PREPARE2READ, rc ) ;
      return rc ;
   error:
      readLen = 0 ;
      pieceNum = 0 ;
      goto done ;
   }

}

