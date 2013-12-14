/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = spdCoordDownloader.cpp

   Descriptive Name =

   When/how to use:

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/19/2013  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "spdCoordDownloader.hpp"
#include "rtnCoordCommands.hpp"
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"
#include "catDef.hpp"
#include "rtn.hpp"
#include "msgMessage.hpp"
#include "spdTrace.h"

namespace engine
{
   PD_TRACE_DECLARE_FUNCTION ( SDB_SPTCOORDDOWNLOADER_SPDCOORDDOWNLOADER, "_spdCoordDownloader::_spdCoordDownloader" )
   _spdCoordDownloader::_spdCoordDownloader( rtnCoordCommand *command,
                                             _pmdEDUCB *cb )
   :_contextID(-1),
    _command(command),
    _cb( cb ),
    _rtnCB(NULL)
   {
      PD_TRACE_ENTRY ( SDB_SPTCOORDDOWNLOADER_SPDCOORDDOWNLOADER ) ;
      _rtnCB =  pmdGetKRCB()->getRTNCB() ;
      PD_TRACE_EXIT ( SDB_SPTCOORDDOWNLOADER_SPDCOORDDOWNLOADER ) ;
   }

   _spdCoordDownloader::~_spdCoordDownloader()
   {
      if ( -1 != _contextID )
      {
         _rtnCB->contextDelete( _contextID, _cb ) ;
      }
   }

   INT32 _spdCoordDownloader::download( const BSONObj &matcher )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != _cb, "impossible" )
      SDB_ASSERT( NULL != _command, "impossible" )

      CHAR *msg = NULL ;
      INT32 bufSize = 0 ;
      CHAR *resBuf = NULL ;
      MsgOpReply reply ;

      if ( -1 != _contextID )
      {
         PD_LOG( PDERROR, "context was already been opened" ) ;
         SDB_ASSERT( FALSE, "impossible" )
         rc = SDB_SYS ;
         goto error ;
      }

      rc = msgBuildQueryMsg( &msg, &bufSize, CAT_PROCEDURES_COLLECTION,
                             0, 0, 0, -1, &matcher , NULL, NULL, NULL) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to build query msg:%d", rc ) ;
         goto error ;
      }

      rc = _command->queryOnCatalog( msg, bufSize, &resBuf,
                                     _cb, reply, MSG_BS_QUERY_REQ ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to query on catalog:%d", rc ) ;
         goto error ;
      }

      SDB_ASSERT( -1 != reply.contextID, "impossible" )
      _contextID = reply.contextID ;
   done:
      if ( NULL != msg )
      {
         SDB_OSS_FREE(msg) ;
      }
      if ( NULL != resBuf )
      {
         SDB_OSS_FREE(resBuf) ;
      }
      return rc ;
   error:
      goto done ;
   }

   INT32 _spdCoordDownloader::next( BSONObj &func )
   {
      INT32 rc = SDB_OK ;
      INT64 startingPos = 0 ;
      rc = rtnGetMore( _contextID, 1, _context, startingPos,
                       _cb, _rtnCB ) ;
      if ( SDB_OK == rc )
      {
         rc = _context.nextObj( func ) ;
         if ( SDB_OK == rc )
         {
            goto done ;
         }
         else
         {
            PD_LOG( PDERROR, "contextbuf should not return err"
                             " when getmore return ok" ) ;
            SDB_ASSERT( FALSE, "impossible" )
            rc = SDB_SYS ;
            goto error ;
         }
      }
      else if ( SDB_DMS_EOC == rc )
      {
         _contextID = -1 ;
         goto error ;
         /// context was freed in getmore.
      }
      else
      {
         PD_LOG( PDERROR, "failed to getmore from context[%lld], rc=%d",
                 _contextID, rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      if ( -1 != _contextID )
      {
         _rtnCB->contextDelete( _contextID, _cb ) ;
      }
      goto done ;
   }
}
