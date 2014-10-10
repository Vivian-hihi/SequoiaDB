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

   Source File Name = rtnCoordLobStream.hpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/10/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef RTN_COORDLOBSTREAM_
#define RTN_COORDLOBSTREAM_

#include "rtnLobStream.hpp"
#include "msg.h"
#include "rtnCoordLobDispatcher.hpp"

namespace engine
{
   class _rtnCoordLobStream : public _rtnLobStream
   {
   public:
      _rtnCoordLobStream() ;
      virtual ~_rtnCoordLobStream() ;

   public:
      virtual _dmsStorageUnit *getSU()
      {
         return NULL ;
      }

   private:
      virtual INT32 _prepare( const CHAR *fullName,
                              const bson::OID &oid,
                              INT32 mode,
                              _pmdEDUCB *cb ) ;

      virtual INT32 _queryLobMeta( _pmdEDUCB *cb,
                                   _dmsLobMeta &meta ) ;

      virtual INT32 _ensureLob( _pmdEDUCB *cb,
                                _dmsLobMeta &meta,
                                BOOLEAN &isNew ) ;

      virtual INT32 _getLobPageSize( INT32 &pageSize ) ;

      virtual INT32 _write( const _dmsLobRecord &record,
                            _pmdEDUCB *cb ) ;

      virtual INT32 _writev( const _dmsLobRecord *pieces,
                             UINT32 cnt,
                             _pmdEDUCB *cb,
                             UINT32 &succNum ) ;

      virtual INT32 _readv( const _dmsLobRecord *pieces,
                            UINT32 cnt,
                            _pmdEDUCB *cb,
                            UINT32 totalLen ) ; 

      virtual INT32 _completeLob( const _dmsLobMeta &meta,
                                  _pmdEDUCB *cb ) ;
 
      virtual INT32 _rollback( _pmdEDUCB *cb ) ;

      virtual INT32 _queryAndInvalidateMetaData( _pmdEDUCB *cb,
                                                 _dmsLobMeta &meta ) ;

      virtual INT32 _removev( const _dmsLobRecord *pieces,
                              UINT32 cnt,
                              _pmdEDUCB *cb ) ;

      virtual INT32 _close( _pmdEDUCB *cb ) ;

   private:
      INT32 _openSubStreams( const CHAR *fullName,
                             const bson::OID &oid,
                             INT32 mode,
                             _pmdEDUCB *cb ) ;

      INT32 _openMainStream( const CHAR *fullName,
                             const bson::OID &oid,
                             INT32 mode,
                             _pmdEDUCB *cb ) ;

      INT32 _openOtherStreams( const CHAR *fullName,
                               const bson::OID &oid,
                               INT32 mode,
                               const CoordGroupList &gpLst,
                               _pmdEDUCB *cb ) ;

      INT32 _extractMeta( const MsgOpReply *header,
                          bson::BSONObj &obj ) ;

      INT32 _closeSubStreams( _pmdEDUCB *cb ) ;

      INT32 _closeSubStreamsWithException( _pmdEDUCB *cb ) ;

      INT32 _push2Pool( _pmdEDUCB *cb ) ;

   private:
      struct subStream
      {
         SINT64 contextID ;
         MsgRouteID id ;

         subStream()
         :contextID( -1 )
         {
            id.value = MSG_INVALID_ROUTEID ;
         }        

         subStream( SINT64 context, MsgRouteID route )
         :contextID( context ),
          id( route )
         {

         }
      } ;

      typedef std::map<UINT32, subStream> SUB_STREAMS ;

      void _add2Subs( UINT32 groupID, SINT64 contextID, MsgRouteID id )
      {
         _subs[groupID] = subStream( contextID, id ) ;
         return ;
      }

      INT32 _updateCataInfo( BOOLEAN refresh,
                             _pmdEDUCB *cb ) ;

   private:
      rtnCoordLobDispatcher _dispatcher ;
      CoordCataInfoPtr _cataInfo ;
      SUB_STREAMS _subs;
      bson::BSONObj _metaObj ;
      UINT32 _metaGroup ;
   } ;
   typedef class _rtnCoordLobStream rtnCoordLobStream ;
}
#endif

