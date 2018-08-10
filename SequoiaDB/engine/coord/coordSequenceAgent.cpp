/*******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = coordSequenceAgent.cpp

   Descriptive Name = Coordinator Sequence Agent

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          08/06/2018  David Li  Initial Draft

   Last Changed =

*******************************************************************************/
#include "coordSequenceAgent.hpp"
#include "coordCommandSequence.hpp"
#include "coordRemoteSession.hpp"
#include "coordCB.hpp"
#include "catGTSDef.hpp"
#include "msgMessage.hpp"
#include "msgMessageFormat.hpp"
#include "msgDef.h"
#include "pmdEDU.hpp"
#include "../bson/bsonobj.h"
#include "pdTrace.hpp"
#include "coordTrace.hpp"
#include "pd.hpp"

using namespace bson ;

namespace engine
{
   class _coordSequence: public SDBObject
   {
   public:
      _coordSequence( const std::string& name )
      {
         _name = name ;
         _nextValue = 0 ;
         _acquireSize = 0 ;
         _increment = 0 ;
      }
      ~_coordSequence() {}

   public:
      OSS_INLINE const std::string& name() const { return _name ; }
      OSS_INLINE bson::OID oid() const { return _oid ; }
      OSS_INLINE INT64 nextValue() const { return _nextValue ; }
      OSS_INLINE INT32 acquireSize() const { return _acquireSize ; }
      OSS_INLINE INT32 increment() const { return _increment ; }

      OSS_INLINE void setOid( OID oid )
      {
         _oid = oid ;
      }
      OSS_INLINE void setNextValue( INT64 nextValue )
      {
         _nextValue = nextValue ;
      }
      OSS_INLINE void setAcquireSize( INT32 acquireSize )
      {
         _acquireSize = acquireSize ;
      }
      OSS_INLINE void setIncrement( INT32 increment )
      {
         _increment = increment ;
      }
      OSS_INLINE void decreaseAcquireSize()
      {
         _acquireSize-- ;
      }

      void copyFrom( const _coordSequence& other )
      {
         // do not change name
         _oid = other._oid ;
         _nextValue = other._nextValue ;
         _acquireSize = other._acquireSize ;
         _increment = other._increment ;
      }

   private:
      std::string    _name ;
      bson::OID      _oid ;
      INT64          _nextValue ;
      INT32          _acquireSize ;
      INT32          _increment ;
   } ;
   typedef _coordSequence coordSequence ;

   _coordSequenceAgent::_coordSequenceAgent()
   {
      _resource = NULL ;
   }

   _coordSequenceAgent::~_coordSequenceAgent()
   {
      clear() ;
   }

   INT32 _coordSequenceAgent::init( _coordResource* resource )
   {
      _resource = resource ;
      return SDB_OK ;
   }

   void _coordSequenceAgent::fini()
   {
      clear() ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORD_SEQ_AGENT_GET_NEXT_VALUE, "_coordSequenceAgent::getNextValue" )
   INT32 _coordSequenceAgent::getNextValue( const std::string& name, INT64& nextValue, _pmdEDUCB* eduCB )
   {
      INT32 rc = SDB_OK ;
      _coordSequence* cache = NULL ;
      _coordSequence* sequence = NULL ;
      PD_TRACE_ENTRY ( SDB_COORD_SEQ_AGENT_GET_NEXT_VALUE ) ;

      COORD_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
      BUCKET_XLOCK( bucket ) ;

      COORD_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
      if ( bucket.end() != iter )
      {
         cache = (*iter).second ;
      }
      else
      {
         _coordSequence seq = _coordSequence( name ) ;

         rc = _acquireSequence( seq, eduCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to acquire sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         sequence = SDB_OSS_NEW _coordSequence( name ) ;
         if ( NULL == sequence )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to alloc sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }

         sequence->copyFrom( seq ) ;

         try
         {
            bucket.insert( COORD_SEQ_MAP::value_type( name, sequence ) ) ;
         }
         catch( std::exception& e )
         {
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Failed to insert sequence[%s] to cache", name.c_str() ) ;
            goto error ;
         }

         cache = sequence ;
         sequence = NULL ;
      }

      SDB_ASSERT( cache->acquireSize() >= 0, "AcquireSize should >= 0" ) ;

      if ( cache->acquireSize() == 0 )
      {
         rc = _acquireSequence( *cache, eduCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to acquire sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            if ( SDB_SEQUENCE_NOT_EXIST == rc )
            {
               // remove cache if sequence not exist
               bucket.erase( name ) ;
               SDB_OSS_DEL( cache ) ;
               cache = NULL ;
            }
            goto error ;
         }

         if ( cache->acquireSize() == 0 )
         {
            SDB_ASSERT( FALSE, "AcquireSize == 0" ) ;
            rc = SDB_SYS ;
            PD_LOG( PDERROR, "Invalid AcquireSize of sequence[%s], rc=%d",
                    name.c_str(), rc ) ;
            goto error ;
         }
      }

      cache->decreaseAcquireSize() ;
      nextValue = cache->nextValue() ;
      cache->setNextValue( cache->nextValue() + cache->increment() ) ;

   done:
      SAFE_OSS_DELETE( sequence ) ;
      PD_TRACE_EXITRC ( SDB_COORD_SEQ_AGENT_GET_NEXT_VALUE, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORD_SEQ_AGENT_REMOVE_CACHE, "_coordSequenceAgent::removeCache" )
   BOOLEAN _coordSequenceAgent::removeCache( const std::string& name )
   {
      BOOLEAN removed = FALSE ;
      PD_TRACE_ENTRY ( SDB_COORD_SEQ_AGENT_REMOVE_CACHE ) ;

      COORD_SEQ_MAP::Bucket& bucket = _sequenceCache.getBucket( name ) ;
      BUCKET_XLOCK( bucket ) ;

      COORD_SEQ_MAP::map_const_iterator iter = bucket.find( name ) ;
      if ( bucket.end() != iter )
      {
         _coordSequence* cache = (*iter).second ;
         bucket.erase( name ) ;
         SDB_OSS_DEL( cache ) ;
         removed = TRUE ;
      }

      PD_TRACE_EXITRC ( SDB_COORD_SEQ_AGENT_REMOVE_CACHE, removed ) ;
      return removed ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORD_SEQ_AGENT_CLEAR, "_coordSequenceAgent::clear" )
   void _coordSequenceAgent::clear()
   {
      PD_TRACE_ENTRY ( SDB_COORD_SEQ_AGENT_CLEAR ) ;

      for ( COORD_SEQ_MAP::bucket_iterator bucketIt = _sequenceCache.begin() ;
            bucketIt != _sequenceCache.end() ;
            bucketIt++ )
      {
         COORD_SEQ_MAP::Bucket& bucket = *bucketIt ;
         BUCKET_XLOCK( bucket ) ;

         for ( COORD_SEQ_MAP::map_const_iterator it = bucket.begin() ;
               it != bucket.end() ;
               it++ )
         {
            _coordSequence* cache = (*it).second ;
            SDB_OSS_DEL ( cache ) ;
         }

         bucket.clear() ;
      }

      PD_TRACE_EXIT( SDB_COORD_SEQ_AGENT_CLEAR ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORD_SEQ_AGENT__ACQUIRE_SEQ, "_coordSequenceAgent::_acquireSequence" )
   INT32 _coordSequenceAgent::_acquireSequence( _coordSequence& seq, _pmdEDUCB* eduCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_COORD_SEQ_AGENT__ACQUIRE_SEQ ) ;

      coordGroupSession session ;
      pmdSubSession *subSession = NULL ;
      MsgHeader *reply = NULL ;
      BSONObj options ;
      CHAR *pBuffer = NULL ;
      INT32 bufferSize = 0 ;

      SDB_ASSERT( seq.acquireSize() == 0, "AcquireSize should be 0" ) ;

      try
      {
         BSONObjBuilder builder ;
         builder.append( CAT_SEQUENCE_NAME, seq.name() ) ;
         if ( seq.oid().isSet() )
         {
            builder.append( CAT_SEQUENCE_OID, seq.oid() ) ;
         }
         options = builder.obj() ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Failed to build acquire msg options for sequence[%s], exception: %s, rc=%d",
                 seq.name().c_str(), e.what(), rc ) ;
         goto error ;
      }

      rc = session.init( _resource, eduCB ) ;
      if ( rc )
      {
         PD_LOG( PDERROR, "Failed to init coord remote session, rc=%d", rc ) ;
         goto error ;
      }
      session.getGroupSel()->setPrimary( TRUE ) ;
      session.getGroupSel()->setServiceType( MSG_ROUTE_CAT_SERVICE ) ;

      rc = msgBuildSequenceAcquireMsg( &pBuffer, &bufferSize, 0, options, eduCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG ( PDERROR, "Failed to build acquire sequence request, rc=%d",
                  rc ) ;
         goto error ;
      }

   retry:
      session.getSession()->clearSubSession() ;
      rc = session.sendMsg( (MsgHeader*)pBuffer, CATALOG_GROUPID,
                            NULL, &subSession ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to send acquire sequence msg, rc=%d", rc ) ;
         goto error ;
      }

      // recv reply
      rc = session.getSession()->waitReply1( TRUE ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to wait acquire sequence reply from catalog group, rc=%d",
                 rc ) ;
         goto error ;
      }

      reply = subSession->getRspMsg() ;
      rc = _processAcquireReply( reply, seq ) ;
      if ( SDB_OK != rc )
      {
         coordGroupSessionCtrl* groupCtrl = session.getGroupCtrl() ;
         UINT32 primaryID = ((MsgOpReply*)reply)->startFrom ;

         if ( groupCtrl->canRetry( rc, reply->routeID,
                                   primaryID, TRUE, TRUE ) )
         {
            groupCtrl->incRetry() ;
            goto retry ;
         }

         PD_LOG( PDERROR, "Failed to process acquire sequence reply, rc=%d",
                 rc ) ;
         goto error ;
      }

   done:
      if ( pBuffer )
      {
         msgReleaseBuffer( pBuffer, eduCB ) ;
         bufferSize = 0 ;
      }
      PD_TRACE_EXITRC ( SDB_COORD_SEQ_AGENT__ACQUIRE_SEQ, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORD_SEQ_AGENT__PROCESS_ACQUIRE_REPLY, "_coordSequenceAgent::_processAcquireReply" )
   INT32 _coordSequenceAgent::_processAcquireReply( MsgHeader* msg, _coordSequence& seq )
   {
      INT32 rc = SDB_OK ;
      MsgOpReply *reply = ( MsgOpReply* )msg ;
      BSONObj obj ;
      BSONElement ele ;
      OID oid ;
      INT64 nextValue = 0 ;
      INT32 acquireSize = 0 ;
      INT32 increment = 0 ;

      PD_TRACE_ENTRY ( SDB_COORD_SEQ_AGENT__PROCESS_ACQUIRE_REPLY ) ;
      SDB_ASSERT( -1 == reply->contextID, "ContextID must be -1" ) ;

      rc = reply->flags ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Recieve error reply for acquire sequence "
                 "request from node[%s], flag=%d",
                 routeID2String( msg->routeID ).c_str(), rc ) ;
         goto error ;
      }

      rc = msgExtractSequenceAcquireReply( (CHAR*)msg, obj ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to extract reply for acquire sequence "
                 "request from node[%s], rc=%d",
                 routeID2String( msg->routeID ).c_str(), rc ) ;
         goto error ;
      }

      // CAT_SEQUENCE_NAME
      ele = obj.getField( CAT_SEQUENCE_NAME ) ;
      if ( String == ele.type() )
      {
         std::string name = ele.String() ;
         if ( name != seq.name() )
         {
            rc = SDB_INVALIDARG ;
            PD_LOG( PDERROR, "Invalid sequence name[%s], expected[%s]",
                    name.c_str(), seq.name().c_str() ) ;
            goto error ;
         }
      }
      else if ( EOO == ele.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Missing option[%s]", CAT_SEQUENCE_NAME ) ;
         goto error ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid type(%d) for option[%s]",
                 ele.type(), CAT_SEQUENCE_NAME ) ;
         goto error ;
      }

      // CAT_SEQUENCE_OID
      ele = obj.getField( CAT_SEQUENCE_OID ) ;
      if ( jstOID == ele.type() )
      {
         oid = ele.OID() ;
      }
      else if ( EOO == ele.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Missing option[%s]", CAT_SEQUENCE_OID ) ;
         goto error ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid type(%d) for option[%s]",
                 ele.type(), CAT_SEQUENCE_OID ) ;
         goto error ;
      }

      // CAT_SEQUENCE_NEXT_VALUE
      ele = obj.getField( CAT_SEQUENCE_NEXT_VALUE ) ;
      if ( NumberLong == ele.type() )
      {
         nextValue = ele.numberLong() ;
      }
      else if ( EOO == ele.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Missing option[%s]", CAT_SEQUENCE_NEXT_VALUE ) ;
         goto error ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid type(%d) for option[%s]",
                 ele.type(), CAT_SEQUENCE_NEXT_VALUE ) ;
         goto error ;
      }

      // CAT_SEQUENCE_ACQUIRE_SIZE
      ele = obj.getField( CAT_SEQUENCE_ACQUIRE_SIZE ) ;
      if ( NumberInt == ele.type() )
      {
         acquireSize = ele.numberInt() ;
      }
      else if ( EOO == ele.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Missing option[%s]", CAT_SEQUENCE_ACQUIRE_SIZE ) ;
         goto error ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid type(%d) for option[%s]",
                 ele.type(), CAT_SEQUENCE_ACQUIRE_SIZE ) ;
         goto error ;
      }

      // CAT_SEQUENCE_INCREMENT
      ele = obj.getField( CAT_SEQUENCE_INCREMENT ) ;
      if ( NumberInt == ele.type() )
      {
         increment = ele.numberInt() ;
      }
      else if ( EOO == ele.type() )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Missing option[%s]", CAT_SEQUENCE_INCREMENT ) ;
         goto error ;
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "Invalid type(%d) for option[%s]",
                 ele.type(), CAT_SEQUENCE_INCREMENT ) ;
         goto error ;
      }

      if ( seq.oid().isSet() && seq.oid() != oid )
      {
         PD_LOG( PDWARNING, "Mismatch oid(%s) for sequence[%s, %s]",
                 oid.str().c_str(), seq.name().c_str(), seq.oid().str().c_str() ) ;
      }
      seq.setOid( oid ) ;
      seq.setNextValue( nextValue ) ;
      seq.setAcquireSize( acquireSize ) ;
      seq.setIncrement( increment ) ;

   done:
      PD_TRACE_EXITRC ( SDB_COORD_SEQ_AGENT__PROCESS_ACQUIRE_REPLY, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_COORD_SEQ_INVALIDATE_CACHE, "coordInvalidateSequenceCache" )
   INT32 coordSequenceInvalidateCache( const std::string& sequenceName, _pmdEDUCB* eduCB )
   {
      INT32 rc = SDB_OK ;
      BSONObj obj ;
      CHAR* buf = NULL ;
      INT32 bufSize = 0 ;
      INT64 contextID = -1 ;
      PD_TRACE_ENTRY ( SDB_COORD_SEQ_INVALIDATE_CACHE ) ;

      _coordCMDInvalidateSequenceCache invalidator ;

      SDB_ASSERT( NULL != eduCB, "eduCB can't null" ) ;

      try
      {
         obj = BSON( FIELD_NAME_SEQUENCE_NAME << sequenceName ) ;
      }
      catch( std::exception& e )
      {
         rc = SDB_SYS ;
         PD_LOG( PDERROR, "Unexcepted exception: %s", e.what() ) ;
         goto error ;
      }

      rc = msgBuildSequenceInvalidateCacheMsg( &buf, &bufSize, sequenceName.c_str(), 0, eduCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to build sequence invalidate cache msg, rc=%d", rc ) ;
         goto error ;
      }

      rc = invalidator.init( sdbGetCoordCB()->getResource(), eduCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to init sequence invalidate cache command, rc=%d", rc ) ;
         goto error ;
      }

      rc = invalidator.execute( (MsgHeader*)buf, eduCB, contextID, NULL ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "Failed to execute sequence invalidate cache command, rc=%d", rc ) ;
         goto error ;
      }

   done:
      if ( NULL != buf )
      {
         msgReleaseBuffer( buf, eduCB ) ;
      }
      PD_TRACE_EXITRC ( SDB_COORD_SEQ_INVALIDATE_CACHE, rc ) ;
      return rc ;
   error:
      goto done ;
   }
}

