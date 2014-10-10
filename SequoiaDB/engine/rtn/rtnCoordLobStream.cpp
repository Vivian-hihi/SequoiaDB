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

   Source File Name = rtnCoordLobStream.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/10/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnCoordLobStream.hpp"
#include "rtnTrace.hpp"
#include "msgDef.hpp"
#include "rtnCoordCommon.hpp"
#include "pmd.hpp"
#include "msgMessage.hpp"

#define RTN_COORD_LOB_GET_SUBSTREAM( groupID, s ) \
        do\
        {\
           SUB_STREAMS::const_iterator itr = _subs.find( groupID ) ;\
           if ( _subs.end() == itr )\
           {\
              PD_LOG( PDERROR, "group:%d is not in sub streams", groupID ) ;\
              rc = SDB_SYS ;\
              goto error ;\
           }\
           s = &( itr->second ) ;\
        } while ( FALSE )

namespace engine
{
   typedef _rtnCoordLobDispatcher::msgOptions MSG_OPTIONS ;

   _rtnCoordLobStream::_rtnCoordLobStream()
   :_metaGroup( 0 )
   {

   }

   _rtnCoordLobStream::~_rtnCoordLobStream()
   {

   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__PREPARE, "_rtnCoordLobStream::_prepare" )
   INT32 _rtnCoordLobStream::_prepare( const CHAR *fullName,
                                       const bson::OID &oid,
                                       INT32 mode,
                                       _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__PREPARE ) ;

      rc = _updateCataInfo( FALSE, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to update catalog info of:%s, rc:%d",
                 fullName, rc ) ;
         goto error ;
      }

      rc = _openSubStreams( fullName, oid, mode, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open sub streams:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__PREPARE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__UPDATECATAINFO, "_rtnCoordLobStream::_openSubStreams" )
   INT32 _rtnCoordLobStream::_updateCataInfo( BOOLEAN refresh,
                                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__UPDATECATAINFO ) ;
      rc = rtnCoordGetCataInfo( cb, getFullName(), refresh, _cataInfo ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get catalog info of:%s, rc:%d",
                 getFullName(), rc ) ;
         goto error ;
      }

      if ( _cataInfo->isMainCL() )
      {
         PD_LOG( PDERROR, "can not open a lob in main cl" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else if ( _cataInfo->isRangeSharded() )
      {
         PD_LOG( PDERROR, "can not open a lob in range sharded cl" ) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
      else
      {
         rc = _cataInfo->getLobGropuID( getOID(),
                                     DMS_LOB_META_SEQUENCE,
                                     _metaGroup ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get meta group:%d", rc ) ;
            goto error ;
         }    
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__UPDATECATAINFO, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__OPENSUBSTREAMS, "_rtnCoordLobStream::_openSubStreams" )
   INT32 _rtnCoordLobStream::_openSubStreams( const CHAR *fullName,
                                              const bson::OID &oid,
                                              INT32 mode,
                                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__OPENSUBSTREAMS ) ;
      CoordGroupList gpLst ;

      rc = _openMainStream( fullName, oid, mode, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open main stream:%d", rc ) ;
         goto error ;
      }

      _cataInfo->getGroupLst( gpLst ) ;
      SDB_ASSERT( 1 == gpLst.count( _metaGroup ), "impossible" ) ;

      /// open other non-main substreams.
      gpLst.erase( _metaGroup ) ;

      rc = _openOtherStreams( fullName, oid, mode, gpLst, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open other streams:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__OPENSUBSTREAMS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__OPENOTHERSTREAMS, "_rtnCoordLobStream::_openOtherStreams" )
   INT32 _rtnCoordLobStream::_openOtherStreams( const CHAR *fullName,
                                                const bson::OID &oid,
                                                INT32 mode,
                                                const CoordGroupList &gpLst,
                                                _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__OPENOTHERSTREAMS ) ;
      BSONObjBuilder builder ;
      BSONObj obj ;
      MSG_OPTIONS options( SDB_LOB_MODE_R != mode, TRUE ) ;

      builder.append( FIELD_NAME_COLLECTION, fullName )
             .append( FIELD_NAME_LOB_OID, oid )
             .append( FIELD_NAME_LOB_OPEN_MODE, mode )
             .appendBool( FIELD_NAME_LOB_IS_MAIN_SHD, FALSE ) ;
      if ( SDB_LOB_MODE_R == mode )
      {
         /// send meta data to every group.
         builder.append( FIELD_NAME_LOB_META_DATA, _metaObj ) ;
      }

      obj = builder.obj() ;

      rc = _dispatcher.createMsg( MSG_BS_LOB_OPEN_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  obj ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      {
      CoordGroupList::const_iterator itr = gpLst.begin() ;
      for ( ; itr != gpLst.end(); ++itr )
      {
         _dispatcher.add( itr->first ) ;
      }
      }

      _dispatcher.addDone() ;
      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      {
      const std::vector<MsgOpReply *> &replyMsgs = _dispatcher.getReplyMsgs() ;
      std::vector<MsgOpReply *>::const_iterator itr = replyMsgs.begin() ;
      for ( ; itr != replyMsgs.end(); ++itr )
      {
         if ( SDB_OK != ( *itr )->flags )
         {
            rc = ( *itr )->flags ;
            PD_LOG( PDERROR, "failed to write lob on node[%d:%hd], rc:%d",
                    ( *itr )->header.routeID.columns.groupID,
                    ( *itr )->header.routeID.columns.nodeID, rc ) ;
            goto error ;
         }

         if ( -1 == ( *itr )->contextID )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "invalid context id" ) ;
            goto error ;
         }

         _add2Subs( ( *itr )->header.routeID.columns.groupID,
                    ( *itr )->contextID,
                    ( *itr )->header.routeID ) ;
      }
      }
   done:
      _dispatcher.clear() ;
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__OPENOTHERSTREAMS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__OPENMAINSTREAM, "_rtnCoordLobStream::_openMainStream" )
   INT32 _rtnCoordLobStream::_openMainStream( const CHAR *fullName,
                                              const bson::OID &oid,
                                              INT32 mode,
                                              _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__OPENMAINSTREAM ) ;
      BSONObjBuilder builder ;
      BSONObj obj ;
      MSG_OPTIONS options( SDB_LOB_MODE_R != mode, TRUE ) ;
      const MsgOpReply *reply = NULL ;

      builder.append( FIELD_NAME_COLLECTION, fullName )
             .append( FIELD_NAME_LOB_OID, oid )
             .append( FIELD_NAME_LOB_OPEN_MODE, mode )
             .appendBool( FIELD_NAME_LOB_IS_MAIN_SHD, TRUE ) ;
      obj = builder.obj() ;

   retry:
      _dispatcher.clear() ;
      rc = _dispatcher.createMsg( MSG_BS_LOB_OPEN_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  obj ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      _dispatcher.add( _metaGroup ).addDone() ;

      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      reply = _dispatcher.getFirstReply() ;
      if ( NULL == reply )
      {
         PD_LOG( PDERROR, "we got nothing from msg queue" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_CLS_COORD_NODE_CAT_VER_OLD == reply->flags )
      {
         PD_LOG( PDEVENT, "our version is old, update catalog again" ) ;

         rc = _updateCataInfo( TRUE, cb ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to upate catalog info of:%s, rc:%d",
                    getFullName(), rc ) ;
            goto error ;
         }
         else
         {
            goto retry ;
         }
      }
      else if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "failed to open lob on data node:%d", rc ) ;
         goto error ;
      }
      else if ( -1 == reply->contextID )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "invalid context id" ) ;
         goto error ;
      }
      else if ( SDB_LOB_MODE_R == mode ||
                SDB_LOB_MODE_REMOVE == mode )
      {
         rc = _extractMeta( reply, _metaObj ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to extract meta data from reply msg:%d", rc ) ;
            goto error ;
         }
      }
      else
      {
         /// do nothing.
      }

      _add2Subs( reply->header.routeID.columns.groupID,
                 reply->contextID, reply->header.routeID ) ;
   done:
      _dispatcher.clear() ;
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__OPENMAINSTREAM, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__QUERYMETA, "_rtnCoordLobStream::_queryLobMeta" )
   INT32 _rtnCoordLobStream::_queryLobMeta( _pmdEDUCB *cb,
                                            _dmsLobMeta &meta )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__QUERYMETA ) ;

      try
      { 
         BSONElement ele = _metaObj.getField( FIELD_NAME_LOB_SIZE ) ;
         if ( NumberLong != ele.type() )
         {
            PD_LOG( PDERROR, "invalid meta obj:%s",
                    _metaObj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         meta._lobLen = ele.Long() ;

         ele = _metaObj.getField( FIELD_NAME_LOB_CREATTIME ) ;
         if ( NumberLong != ele.type() )
         {
            PD_LOG( PDERROR, "invalid meta obj:%s",
                    _metaObj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }
         meta._createTime = ele.Long() ;

         meta._status = DMS_LOB_COMPLETE ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__QUERYMETA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnCoordLobStream::_ensureLob( _pmdEDUCB *cb,
                                         _dmsLobMeta &meta,
                                         BOOLEAN &isNew )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( SDB_LOB_MODE_CREATEONLY == _getMode(), "should not hit here" ) ;
      isNew = TRUE ;
      return rc ;
   }

   INT32 _rtnCoordLobStream::_getLobPageSize( INT32 &pageSize )
   {
      INT32 rc = SDB_OK ;
      pageSize = DMS_PAGE_SIZE256K ;
/*
      try
      {
         BSONElement ele = _metaObj.getField( FIELD_NAME_LOB_PAGE_SIZE ) ;
         if ( NumberInt != ele.type() )
         {
            PD_LOG( PDERROR, "invalid meta obj:%s",
                    _metaObj.toString( FALSE, TRUE ).c_str() ) ;
            rc = SDB_SYS ;
            goto error ;
         }

         pageSize = ele.Int() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;      
      }
*/
      return rc ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__WRITE, "_rtnCoordLobStream::_write" )
   INT32 _rtnCoordLobStream::_write( const _dmsLobRecord &record,
                                     _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__WRITE ) ;
      UINT32 groupID = 0 ;
      MSG_OPTIONS options( FALSE, FALSE ) ;
      _MsgLobTuple tuple ;
      tuple.columns.len = record._dataLen ;
      tuple.columns.sequence = record._sequence ;
      tuple.columns.offset = record._offset ; /// offset in piece
      const subStream *sub = NULL ;
      const MsgOpReply *reply = NULL ;

      rc = _cataInfo->
                   getLobGropuID( *( record._oid ),
                                  record._sequence,
                                  groupID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get destination:%d", rc ) ;
         goto error ;
      }

      RTN_COORD_LOB_GET_SUBSTREAM( groupID, sub ) ;

      rc = _dispatcher.createMsg( MSG_BS_LOB_WRITE_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  BSONObj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      _dispatcher.add( sub->id, tuple.data, sizeof( tuple ) )
                 .add( sub->id, record._data, record._dataLen)
                 .addDone() ;

      _dispatcher.setContextID( sub->id, sub->contextID ) ;

      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      reply = _dispatcher.getFirstReply() ;
      if ( NULL == reply )
      {
         PD_LOG( PDERROR, "we got nothing from msg queue" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "failed to write lob on data node:[%d:%d], rc%d",
                 reply->header.routeID.columns.groupID,
                 reply->header.routeID.columns.nodeID, rc ) ;
         goto error ;
      }
       
   done:
      _dispatcher.clear() ;
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__WRITE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__WRITEV, "_rtnCoordLobStream::_writev" )
   INT32 _rtnCoordLobStream::_writev( const _dmsLobRecord *pieces,
                                      UINT32 cnt,
                                      _pmdEDUCB *cb,
                                      UINT32 &succNum )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__WRITEV ) ;
      SDB_ASSERT( cnt <= RTN_LOB_WRITE_PIECE_NUM, "can not over it" ) ;
      MSG_OPTIONS options( FALSE, FALSE ) ;
      _MsgLobTuple tuples[RTN_LOB_WRITE_PIECE_NUM] ;

      _dispatcher.clear() ;
      rc = _dispatcher.createMsg( MSG_BS_LOB_WRITE_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  BSONObj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      for ( UINT32 i = 0; i < cnt; ++i )
      {
         const _dmsLobRecord &piece = pieces[i] ;
         UINT32 groupID = 0 ;
         _MsgLobTuple &tuple = tuples[i] ;
         tuple.columns.len = piece._dataLen ;
         tuple.columns.sequence = piece._sequence ;
         tuple.columns.offset = piece._offset ; /// offset in piece
         const subStream *sub = NULL ;

         rc = _cataInfo->
                   getLobGropuID( *( piece._oid ),
                                  piece._sequence,
                                  groupID ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get destination:%d", rc ) ;
            goto error ;
         }

         RTN_COORD_LOB_GET_SUBSTREAM( groupID, sub ) ;

         _dispatcher.add( sub->id, tuple.data, sizeof( tuple ) )
                 .add( sub->id, piece._data, piece._dataLen) ;
         _dispatcher.setContextID( sub->id, sub->contextID ) ;
      }
      _dispatcher.addDone() ;

      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      {
      const std::vector<MsgOpReply *> &replyMsgs = _dispatcher.getReplyMsgs() ;
      std::vector<MsgOpReply *>::const_iterator itr = replyMsgs.begin() ;
      for ( ; itr != replyMsgs.end(); ++itr )
      {
         if ( SDB_OK != ( *itr )->flags )
         {
            rc = ( *itr )->flags ;
            PD_LOG( PDERROR, "failed to write lob on node[%d:%hd], rc:%d",
                    ( *itr )->header.routeID.columns.groupID,
                    ( *itr )->header.routeID.columns.nodeID, rc ) ;
            goto error ;
         }
      }
      }
   done:
      _dispatcher.clear() ;
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__WRITEV, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__READV, "_rtnCoordLobStream::_readv" )
   INT32 _rtnCoordLobStream::_readv( const _dmsLobRecord *pieces,
                                     UINT32 cnt,
                                     _pmdEDUCB *cb,
                                     UINT32 totalLen )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__READV ) ;
      SDB_ASSERT( cnt <= RTN_LOB_READ_PIECE_NUM, "impossible" ) ;
      MSG_OPTIONS options( FALSE, FALSE ) ;
      _MsgLobTuple tuples[RTN_LOB_READ_PIECE_NUM] ;

      _dispatcher.clear() ;
      rc = _dispatcher.createMsg( MSG_BS_LOB_READ_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  BSONObj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      for ( UINT32 i = 0; i < cnt; ++i )
      {
         const _dmsLobRecord &piece = pieces[i] ;
         UINT32 groupID = 0 ;
         _MsgLobTuple &tuple = tuples[i] ;
         tuple.columns.len = piece._dataLen ;
         tuple.columns.sequence = piece._sequence ;
         tuple.columns.offset = piece._offset ; /// offset in piece
         const subStream *sub = NULL ;

         rc = _cataInfo->
                   getLobGropuID( *( piece._oid ),
                                  piece._sequence,
                                  groupID ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get destination:%d", rc ) ;
            goto error ;
         }

         RTN_COORD_LOB_GET_SUBSTREAM( groupID, sub ) ;

         _dispatcher.add( sub->id, tuple.data, sizeof( tuple ) ) ;
         _dispatcher.setContextID( sub->id, sub->contextID ) ;
      }
      _dispatcher.addDone() ;

      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      rc = _push2Pool( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to resort data:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__READV, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__PUSH2POOL, "_rtnCoordLobStream::_push2Pool" )
   INT32 _rtnCoordLobStream::_push2Pool( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__PUSH2POOL ) ;

      INT32 pageSz = 0 ;
      rc = _getLobPageSize( pageSz ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get page size of lob:%d", rc ) ;
         goto error ;
      }

      _getPool().clear() ;
      {
      const std::vector<MsgOpReply *> &replyMsgs = _dispatcher.getReplyMsgs() ;
      std::vector<MsgOpReply *>::const_iterator itr = replyMsgs.begin() ;
      for ( ; itr != replyMsgs.end(); ++itr )
      {
         const MsgLobTuple *begin = NULL ;
         UINT32 tupleSz = 0 ;
         const MsgLobTuple *curTuple = NULL ;
         const CHAR *data = NULL ;
         BOOLEAN got = FALSE ;

         if ( SDB_OK != ( *itr )->flags )
         {
            rc = ( *itr )->flags ;
            PD_LOG( PDERROR, "failed to read lob on node[%d:%hd], rc:%d",
                    ( *itr )->header.routeID.columns.groupID,
                    ( *itr )->header.routeID.columns.nodeID, rc ) ;
            goto error ;
         }

         rc = msgExtractReadResult( *itr, &begin, &tupleSz ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to extract read result:%d", rc ) ;
            goto error ;
         }

         while ( TRUE )
         {
            rc = msgExtractTuplesAndData( &begin, &tupleSz,
                                          &curTuple, &data, &got ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to extract tuple from msg:%d", rc ) ;
               goto error ;
            }
            else if ( got )
            {
               if ( 0 == curTuple->columns.sequence )
               {
                  PD_LOG( PDERROR, "we should not get sequence 0" ) ;
                  rc = SDB_SYS ;
                  goto error ;
               }

               rc = _getPool().push( data, curTuple->columns.len,
                                     RTN_LOB_GET_OFFSET_OF_LOB(
                                            pageSz,
                                            curTuple->columns.sequence,
                                            curTuple->columns.offset ) ) ;
               if ( SDB_OK != rc )
               {
                  PD_LOG( PDERROR, "failed to push data to pool:%d", rc ) ;
                  goto error ;
               }
            }
            else
            {
               break ;
            }
         }
      }
      }

      _getPool().pushDone() ;

      {
      const std::vector<MsgOpReply *> &replyMsgs = _dispatcher.getReplyMsgs() ;
      std::vector<MsgOpReply *>::const_iterator itr = replyMsgs.begin() ;
      for ( ; itr != replyMsgs.end(); ++itr )
      {
         _getPool().entrust( ( CHAR * )( *itr ) ) ;
      }
      }

      /// MsgOpReply will be freed by pool. 
      _dispatcher.clear( FALSE ) ;
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__PUSH2POOL, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__COMPLETELOB, "_rtnCoordLobStream::_completeLob" )
   INT32 _rtnCoordLobStream::_completeLob( const _dmsLobMeta &meta,
                                           _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__COMPLETELOB ) ;
      MSG_OPTIONS options( FALSE, FALSE ) ;
      _MsgLobTuple tuple ;
      tuple.columns.len = sizeof( meta ) ;
      tuple.columns.sequence = DMS_LOB_META_SEQUENCE ;
      tuple.columns.offset = 0 ; /// offset in piece
      const subStream *sub = NULL ;
      const MsgOpReply *reply = NULL ;

      RTN_COORD_LOB_GET_SUBSTREAM( _metaGroup, sub ) ;

      _dispatcher.clear() ;
      rc = _dispatcher.createMsg( MSG_BS_LOB_UPDATE_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  BSONObj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      _dispatcher.add( sub->id, tuple.data, sizeof( tuple ) )
                 .add( sub->id, &meta, sizeof( meta ) )
                 .addDone() ;

      _dispatcher.setContextID( sub->id, sub->contextID ) ;

      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      reply = _dispatcher.getFirstReply() ;
      if ( NULL == reply )
      {
         PD_LOG( PDERROR, "we got nothing from msg queue" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      if ( SDB_OK != reply->flags )
      {
         rc = reply->flags ;
         PD_LOG( PDERROR, "failed to write lob on data node:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__COMPLETELOB, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnCoordLobStream::_close( _pmdEDUCB *cb )
   {
      return _closeSubStreams( cb ) ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__ROLLBACK, "_rtnCoordLobStream::_rollback" )
   INT32 _rtnCoordLobStream::_rollback( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__ROLLBACK ) ;
      rc = _closeSubStreamsWithException( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "got error when rollback lob:%d", rc ) ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__ROLLBACK, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__CLOSESUBSTREAMS, "_rtnCoordLobStream::_closeSubStreams" )
   INT32 _rtnCoordLobStream::_closeSubStreams( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__CLOSESUBSTREAMS ) ;
      MSG_OPTIONS options( FALSE, FALSE ) ;

      _dispatcher.clear() ;
      rc = _dispatcher.createMsg( MSG_BS_LOB_CLOSE_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  BSONObj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      {
      SUB_STREAMS::const_iterator itr = _subs.begin() ;
      for ( ; itr != _subs.end(); ++itr )
      {
         _dispatcher.add( itr->second.id ) ;
      }
      }

      {
      SUB_STREAMS::const_iterator itr = _subs.begin() ;
      for ( ; itr != _subs.end(); ++itr )
      {
         _dispatcher.setContextID( itr->second.id,
                                   itr->second.contextID ) ;
      }
      }

      _dispatcher.addDone() ;
      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      {
      const std::vector<MsgOpReply *> &replyMsgs = _dispatcher.getReplyMsgs() ;
      std::vector<MsgOpReply *>::const_iterator itr = replyMsgs.begin() ;
      for ( ; itr != replyMsgs.end(); ++itr )
      {
         if ( SDB_OK != ( *itr )->flags )
         {
            rc = ( *itr )->flags ;
            PD_LOG( PDERROR, "failed to close lob on node[%d:%hd], rc:%d",
                    ( *itr )->header.routeID.columns.groupID,
                    ( *itr )->header.routeID.columns.nodeID, rc ) ;
            goto error ;
         }
      }
      }
      _subs.clear() ;
   done:
      _dispatcher.clear() ;
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__CLOSESUBSTREAMS, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__CLOSESSWITHEXCEP, "_rtnCoordLobStream::__closeSubStreamsWithException" )
   INT32 _rtnCoordLobStream::_closeSubStreamsWithException( _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__CLOSESSWITHEXCEP ) ;
      REQUESTID_MAP sendMap ;
      REPLY_QUE q ;
      netMultiRouteAgent *route = pmdGetKRCB()->getCoordCB()->
                                  getRouteAgent() ;
      MsgOpKillContexts killMsg ;
      killMsg.header.messageLength = sizeof ( MsgOpKillContexts ) ;
      killMsg.header.opCode = MSG_BS_KILL_CONTEXT_REQ ;
      killMsg.header.TID = cb->getTID() ;
      killMsg.header.routeID.value = 0;
      killMsg.ZERO = 0;
      killMsg.numContexts = 1 ;

      SUB_STREAMS::const_iterator itr = _subs.begin() ;
      for ( ; itr != _subs.end(); ++itr )
      {
         killMsg.contextIDs[0] = itr->second.contextID ;
         rc = rtnCoordSendRequestToNode( &killMsg, itr->second.id,
                                         route, cb, sendMap ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to kill sub context on node[%d:%hd], rc:%d",
                    itr->second.id.columns.groupID,
                    itr->second.id.columns.nodeID, rc ) ;
            /// try to rollback all substreams, so do not goto error.
         }
      }

      rc = rtnCoordGetReply( cb, sendMap, q, MSG_BS_KILL_CONTEXT_RES,
                             TRUE, TRUE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply:%d", rc ) ;
         goto error ;
      }
   done:
      while ( !q.empty() )
      {
         CHAR *p = q.front();
         q.pop();
         SAFE_OSS_FREE( p ) ;
      }
      _subs.clear() ;
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__CLOSESSWITHEXCEP, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__EXTRACTMETA, "_rtnCoordLobStream::_extractMeta" )   
   INT32 _rtnCoordLobStream::_extractMeta( const MsgOpReply *header,
                                           bson::BSONObj &metaObj )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__EXTRACTMETA ) ;
      const CHAR *metaRaw = NULL ;

      if ( ( UINT32 )header->header.messageLength <
           ( sizeof( MsgOpReply ) + 5 ) )
      {
         PD_LOG( PDERROR, "invalid msg length:%d",
                 header->header.messageLength ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      metaRaw = ( const CHAR * )header + sizeof( MsgOpReply ) ;
      try
      {
         metaObj = BSONObj( metaRaw ).getOwned() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__EXTRACTMETA, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _rtnCoordLobStream::_queryAndInvalidateMetaData( _pmdEDUCB *cb,
                                                          _dmsLobMeta &meta )
   {
      return _queryLobMeta( cb, meta ) ;
   }

   //PD_TRACE_DECLARE_FUNCTION( SDB_RTNCOORDLOBSTREAM__REMOVEV, "_rtnCoordLobStream::_removev" )
   INT32 _rtnCoordLobStream::_removev( const _dmsLobRecord *pieces,
                                       UINT32 cnt,
                                       _pmdEDUCB *cb )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCOORDLOBSTREAM__REMOVEV ) ;
      SDB_ASSERT( cnt <= RTN_LOB_REMOVE_PIECE_NUM, "can not over it" ) ;
      MSG_OPTIONS options( TRUE, FALSE ) ;
      _dispatcher.clear() ;
      _MsgLobTuple tuples[RTN_LOB_REMOVE_PIECE_NUM] ;

      rc = _dispatcher.createMsg( MSG_BS_LOB_REMOVE_REQ,
                                  _cataInfo->getVersion(),
                                  options,
                                  BSONObj() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to create new msg:%d", rc ) ;
         goto error ;
      }

      for ( UINT32 i = 0; i < cnt; ++i )
      {
         const _dmsLobRecord &piece = pieces[i] ;
         UINT32 groupID = 0 ;
         _MsgLobTuple &tuple = tuples[i] ;
         tuple.columns.len = piece._dataLen ;
         tuple.columns.sequence = piece._sequence ;
         tuple.columns.offset = piece._offset ; /// offset in piece
         const subStream *sub = NULL ;

         rc = _cataInfo->
                   getLobGropuID( *( piece._oid ),
                                  piece._sequence,
                                  groupID ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to get destination:%d", rc ) ;
            goto error ;
         }

         RTN_COORD_LOB_GET_SUBSTREAM( groupID, sub ) ;

         _dispatcher.add( sub->id, tuple.data, sizeof( tuple ) ) ;
         _dispatcher.setContextID( sub->id, sub->contextID ) ;
      }
      _dispatcher.addDone() ;

      rc = _dispatcher.wait4Reply( cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get reply msg:%d", rc ) ;
         goto error ;
      }

      {
      const std::vector<MsgOpReply *> &replyMsgs = _dispatcher.getReplyMsgs() ;
      std::vector<MsgOpReply *>::const_iterator itr = replyMsgs.begin() ;
      for ( ; itr != replyMsgs.end(); ++itr )
      {
         if ( SDB_OK != ( *itr )->flags )
         {
            rc = ( *itr )->flags ;
            PD_LOG( PDERROR, "failed to remove lob on node[%d:%hd], rc:%d",
                    ( *itr )->header.routeID.columns.groupID,
                    ( *itr )->header.routeID.columns.nodeID, rc ) ;
            goto error ;
         }
      }
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNCOORDLOBSTREAM__REMOVEV, rc ) ;
      return rc ;
   error:
      goto done ;
   }

}

