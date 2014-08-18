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

   Source File Name = rtnLob.cpp

   Descriptive Name =

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/31/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "rtnLob.hpp"
#include "dmsStorageUnit.hpp"
#include "dmsLobDef.hpp"
#include "pd.hpp"
#include "rtnContextLob.hpp"
#include "rtnTrace.hpp"

namespace engine
{
   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNOPENLOB, "rtnOpenLob" )
   INT32 rtnOpenLob( const BSONObj &lob,
                     SINT32 flags,
                     BOOLEAN isLocal,
                     _pmdEDUCB *cb,
                     SDB_DPSCB *dpsCB,
                     SINT16 w,
                     SINT64 &contextID,
                     BSONObj &meta )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNOPENLOB ) ;
      rtnContextLob *lobContext = NULL ;
      SDB_RTNCB *rtnCB = sdbGetRTNCB() ;

      rc = rtnCB->contextNew( RTN_CONTEXT_LOB,
                              (rtnContext**)(&lobContext),
                              contextID, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open lob context:%d", rc ) ;
         goto error ;
      }

      SDB_ASSERT( NULL != lobContext, "can not be null" ) ;
      rc = lobContext->open( lob, isLocal, cb, dpsCB ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open lob context:%d", rc ) ;
         goto error ;
      }

      rc = lobContext->getLobMetaData( meta ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get meta data:%d", rc ) ;
         goto error ;
      }

      if ( NULL != dpsCB && 1 < w )
      {
         dpsCB->completeOpr( cb, w ) ;
         cb->resetLsn () ;
      }
   done:
      PD_TRACE_EXITRC( SDB_RTNOPENLOB, rc ) ;
      return rc ;
   error:
      if ( -1 != contextID )
      {
         rtnCB->contextDelete ( contextID, cb ) ;
         contextID = -1 ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNREADLOB, "rtnReadLob" )
   INT32 rtnReadLob( SINT64 contextID,
                     pmdEDUCB *cb,
                     UINT32 len,
                     const CHAR **buf,
                     UINT32 &read )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNREADLOB ) ;
      rtnContextLob *lobContext = NULL ;
      SDB_RTNCB *rtnCB = sdbGetRTNCB() ;
      rtnContextBuf contextBuf ;
      INT64 startPos = 0 ;
      rtnContext *context = rtnCB->contextFind ( contextID ) ;
      if ( NULL == context )
      {
         PD_LOG ( PDERROR, "Context %lld does not exist", contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }
      
      if ( !cb->contextFind ( contextID ) )
      {
         PD_LOG ( PDERROR, "Context %lld does not owned by current session",
                  contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }

      if ( RTN_CONTEXT_LOB != context->getType() )
      {
         PD_LOG( PDERROR, "it is not a lob context, invalid context type:%d"
                 ", contextID:%lld", context->getType(), contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      lobContext = ( rtnContextLob * )context ;
      rc = lobContext->read( len, cb ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_EOF != rc )
         {
            PD_LOG( PDERROR, "failed to read lob:%d", rc ) ;
         }

         goto error ;
      }

      rc = lobContext->getMore( -1, contextBuf, startPos, cb ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get more from context:%d", rc ) ;
         goto error ;
      }

      *buf = contextBuf.data() ;
      read = contextBuf.size() ;
   done:
      PD_TRACE_EXITRC( SDB_RTNREADLOB, rc ) ;
      return rc ;
   error:
      if ( SDB_EOF != rc )
      {
         rtnCB->contextDelete ( contextID, cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNWRITELOB, "rtnWriteLob" )
   INT32 rtnWriteLob( SINT64 contextID,
                      pmdEDUCB *cb,
                      UINT32 len,
                      const CHAR *buf )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNWRITELOB ) ;
      rtnContextLob *lobContext = NULL ;
      SDB_RTNCB *rtnCB = sdbGetRTNCB() ;
      rtnContext *context = rtnCB->contextFind ( contextID ) ;
      if ( NULL == context )
      {
         PD_LOG ( PDERROR, "Context %lld does not exist", contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }

      if ( !cb->contextFind ( contextID ) )
      {
         PD_LOG ( PDERROR, "Context %lld does not owned by current session",
                  contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }

      if ( RTN_CONTEXT_LOB != context->getType() )
      {
         PD_LOG( PDERROR, "it is not a lob context, invalid context type:%d"
                 ", contextID:%lld", context->getType(), contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      lobContext = ( rtnContextLob * )context ;
      rc = lobContext->write( len, buf, cb ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_EOF != rc )
         {
            PD_LOG( PDERROR, "failed to read lob:%d", rc ) ;
         }

         goto error ;
      }      
   done:
      PD_TRACE_EXITRC( SDB_RTNWRITELOB, rc ) ;
      return rc ;
   error:
      if ( -1 != contextID )
      {
         rtnCB->contextDelete ( contextID, cb ) ;
      }
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNCLOSELOB, "rtnCloseLob" )
   INT32 rtnCloseLob( SINT64 contextID,
                     pmdEDUCB *cb )
   {
      
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNCLOSELOB ) ;
      rtnContextLob *lobContext = NULL ;
      SDB_RTNCB *rtnCB = sdbGetRTNCB() ;
      rtnContext *context = rtnCB->contextFind ( contextID ) ;
      if ( NULL == context )
      {
         /// context has been closed.
         goto done ;
      }

      if ( !cb->contextFind ( contextID ) )
      {
         PD_LOG ( PDERROR, "Context %lld does not owned by current session",
                  contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }

      if ( RTN_CONTEXT_LOB != context->getType() )
      {
         PD_LOG( PDERROR, "it is not a lob context, invalid context type:%d"
                 ", contextID:%lld", context->getType(), contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      lobContext = ( rtnContextLob * )context ;
      rc = lobContext->close( cb ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_EOF != rc )
         {
            PD_LOG( PDERROR, "failed to read lob:%d", rc ) ;
         }

         goto error ;
      }
   done:
      if ( -1 != contextID )
      {
         rtnCB->contextDelete ( contextID, cb ) ;
      }
      PD_TRACE_EXITRC( SDB_RTNCLOSELOB, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNREMOVELOB, "rtnRemoveLob" )
   INT32 rtnRemoveLob( const BSONObj &meta,
                       SINT32 flags,
                       SINT16 w,
                       _pmdEDUCB *cb,
                       SDB_DPSCB *dpsCB )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNREMOVELOB ) ;
      SDB_DMSCB *dmsCB = sdbGetDMSCB() ;
      dmsStorageUnitID suID = DMS_INVALID_CS ;
      const CHAR *clName = NULL ;
      _dmsStorageUnit *su = NULL ;
      _dmsMBContext *mbContext = NULL ;
      BSONElement fullName ;
      BSONElement oidEle ;
      _dmsLobMeta lobMeta ;
      BOOLEAN lockDms = FALSE ;
      BOOLEAN lockMb = FALSE ;
      SINT32 pageSize = 0 ;
      INT32 num = 0 ;
      dmsLobRecord piece ;
      bson::OID oid ;

      fullName = meta.getField( FIELD_NAME_COLLECTION ) ;
      if ( String != fullName.type() )
      {
         PD_LOG( PDERROR, "invalid type of full name:%s",
                 meta.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      oidEle = meta.getField( FIELD_NAME_LOB_OID ) ;
      if ( jstOID != oidEle.type() )
      {
         PD_LOG( PDERROR, "invalid type of full oid:%s",
                 meta.toString( FALSE, TRUE ).c_str() ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      oid = oidEle.OID() ;

      rc = rtnResolveCollectionNameAndLock( fullName.valuestr(), dmsCB,
                                            &su, &clName, suID ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to resolve collection name:%s",
                 fullName.valuestr() ) ;
         goto error ;
      }

      rc = su->data()->getMBContext( &mbContext, clName, -1 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to resolve collection name:%s",
                 clName ) ;
         goto error ;
      }

      rc = dmsCB->writable( cb ) ;
      if ( SDB_OK !=rc )
      {
         PD_LOG ( PDERROR, "database is not writable, rc = %d", rc ) ;
         goto error ;
      }
      lockDms = TRUE ;

      rc = mbContext->mbLock( EXCLUSIVE ) ;
      PD_RC_CHECK( rc, PDERROR, "dms mb context lock failed, rc: %d", rc ) ;
      lockMb = TRUE ;

      pageSize = su->lob()->getHeader()->_lobdPageSize ;

      rc = su->lob()->getLobMeta( oid, mbContext, cb,
                                  TRUE, lobMeta ) ;
      if ( SDB_OK != rc && SDB_FNE != rc )
      {
         PD_LOG( PDERROR, "failed to get lob meta:%d", rc ) ;
         goto error ;
      }
      else if ( SDB_FNE == rc )
      {
         rc = SDB_OK ;
         goto done ;
      }
      else if ( !lobMeta.isDone() )
      {
         /// other one is deleting or writing this lob.
         rc = SDB_OK ;
         goto error ;
      }
      else
      {
         lobMeta._status = DMS_LOB_UNCOMPLETE ;
         rc = su->lob()->writeLobMeta( oid, mbContext, cb,
                                       TRUE, lobMeta, FALSE, dpsCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write lob meta:%d", rc ) ;
            goto error ;
         }
         mbContext->mbUnlock() ;
         lockMb = FALSE ;
      }

      RTN_LOB_GET_SEQUENCE_NUM( lobMeta._lobLen, pageSize, num ) ;
      while ( 0 < num )
      {
         --num ;
         piece.set( &oid, num, 0, 0, NULL ) ;
         rc = su->lob()->remove( piece, mbContext, cb,
                                 FALSE, dpsCB ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to remove lob[%s],"
                    "sequence:%d, rc:%d", piece._oid->str().c_str(),
                    piece._sequence, rc ) ;
            goto error ;
         }
      }

      if ( NULL != dpsCB && 1 < w )
      {
         dpsCB->completeOpr( cb, w ) ;
         cb->resetLsn () ;
      }

      PD_LOG( PDEVENT, "remove lob[%s]", oid.str().c_str() ) ;    
   done:
      if ( lockMb )
      {
         mbContext->mbUnlock() ;
      }
      if ( lockDms )
      {
         dmsCB->writeDown() ;
      }
      if ( NULL != su )
      {
         dmsCB->suUnlock ( su->CSID() ) ;
      }
      PD_TRACE_EXITRC( SDB_RTNREMOVELOB, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_RTNGETLOBMETADATA, "rtnGetLobMetaData" )
   INT32 rtnGetLobMetaData( SINT64 contextID,
                            pmdEDUCB *cb, 
                            BSONObj &meta )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY( SDB_RTNGETLOBMETADATA ) ;
      rtnContextLob *lobContext = NULL ;
      SDB_RTNCB *rtnCB = sdbGetRTNCB() ;
      rtnContext *context = rtnCB->contextFind ( contextID ) ;
      if ( NULL == context )
      {
         PD_LOG ( PDERROR, "Context %lld does not exist", contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }

      if ( !cb->contextFind ( contextID ) )
      {
         PD_LOG ( PDERROR, "Context %lld does not owned by current session",
                  contextID ) ;
         rc = SDB_RTN_CONTEXT_NOTEXIST ;
         goto error ;
      }

      if ( RTN_CONTEXT_LOB != context->getType() )
      {
         PD_LOG( PDERROR, "it is not a lob context, invalid context type:%d"
                 ", contextID:%lld", context->getType(), contextID ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      lobContext = ( rtnContextLob * )context ;
      rc = lobContext->getLobMetaData( meta ) ;
      if ( SDB_OK != rc )
      {
         if ( SDB_EOF != rc )
         {
            PD_LOG( PDERROR, "failed to get lob meta data:%d", rc ) ;
         }

         goto error ;
      }      
   done:
      PD_TRACE_EXITRC( SDB_RTNGETLOBMETADATA, rc ) ;
      return rc ;
   error:
      if ( -1 != contextID )
      {
         rtnCB->contextDelete ( contextID, cb ) ;
      }
      goto done ;
   }
}

