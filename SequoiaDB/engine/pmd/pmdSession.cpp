/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdSession.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/04/2014  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "pmdSession.hpp"
#include "pmdEDU.hpp"
#include "pmdCommon.hpp"
#include "msgMessage.hpp"
#include "pmd.hpp"
#include "pmdCB.hpp"

namespace engine
{
   const UINT32 SESSION_SOCKET_DFT_TIMEOUT = 10 ;

   /*
      _pmdSession implement
   */

   // message map define
   BEGIN_OBJ_MSG_MAP( _pmdSession, _clsObjBase )

   END_OBJ_MSG_MAP()

   _pmdSession::_pmdSession( _pmdEDUCB *cb, SOCKET fd )
   :_socket( &fd, SESSION_SOCKET_DFT_TIMEOUT )
   {
      _pEDUCB  = cb ;
      _eduID   = cb->getID() ;
      _pBuff   = NULL ;
      _buffLen = 0 ;

      _socket.disableNagle() ;
   }

   _pmdSession::~_pmdSession()
   {
      // release buff
      if ( _pBuff )
      {
         SDB_OSS_FREE( _pBuff ) ;
         _pBuff = NULL ;
      }
      _buffLen = 0 ;
   }

   const CHAR* _pmdSession::sessionName ()
   {
      if ( _pEDUCB )
      {
         return _pEDUCB->getName() ;
      }
      return "Unknow" ;
   }

   INT32 _pmdSession::getBuff( INT32 len, CHAR **ppBuff, INT32 &buffLen )
   {
      INT32 rc = SDB_OK ;

      if ( _buffLen < len )
      {
         if ( _pBuff )
         {
            SDB_OSS_FREE( _pBuff ) ;
            _pBuff = NULL ;
         }
         _buffLen = 0 ;

         len = ossRoundUpToMultipleX( len, SDB_PAGE_SIZE ) ;
         _pBuff = ( CHAR* )SDB_OSS_MALLOC( len ) ;
         if ( !_pBuff )
         {
            rc = SDB_OOM ;
            PD_LOG( PDERROR, "Failed to alloc memory in session[%lld], "
                    "size = %d", sessionID(), len ) ;
            goto error ;
         }
      }

      *ppBuff = _pBuff ;
      buffLen = _buffLen ;

   done:
      return rc ;
   error:
      goto done ;
   }

   void _pmdSession::disconnect()
   {
      _socket.close() ;
   }

   INT32 _pmdSession::sendMsg( const CHAR * pData, INT32 size )
   {
      INT32 rc = SDB_OK ;
      INT32 sentSize = 0 ;
      INT32 totalSentSize = 0 ;

      while ( TRUE )
      {
         if ( _pEDUCB->isForced () )
         {
            rc = SDB_APP_FORCED ;
            goto done ;
         }
         rc = _socket.send ( &pData[totalSentSize], size-totalSentSize,
                             sentSize ) ;
         totalSentSize += sentSize ;
         if ( SDB_TIMEOUT == rc )
         {
            continue ;
         }
         break ;
      }

   done :
      if ( totalSentSize > 0 )
      {
         pmdGetKRCB()->getMonDBCB()->svcNetOutAdd( totalSentSize ) ;
      }
      return rc ;
   }

   /*
      _pmdLocalSession implement
   */

   BEGIN_OBJ_MSG_MAP( _pmdLocalSession, pmdSession )

   END_OBJ_MSG_MAP()

   _pmdLocalSession::_pmdLocalSession( _pmdEDUCB *cb, SOCKET fd )
   :pmdSession( cb, fd )
   {
      _authOK  = FALSE ;
   }

   _pmdLocalSession::~_pmdLocalSession()
   {
   }

   INT32 _pmdLocalSession::_defaultMsgFunc( NET_HANDLE handle, MsgHeader *msg )
   {
      return SDB_OK ;
   }

   INT32 _pmdLocalSession::_onAuth( MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;

      if ( SDB_ROLE_STANDALONE == pmdGetDBRole() ) // not auth
      {
         _authOK = TRUE ;
         goto done ;
      }
      else
      {
         MsgHeader *pAuthRes = NULL ;
         shardCB *pShard = pmdGetKRCB()->getShardCB() ;
         BOOLEAN hasRetry = FALSE ;

         while ( TRUE )
         {
            rc = pShard->syncSend( msg, CATALOG_GROUPID, TRUE, &pAuthRes ) ;
            if ( SDB_OK != rc )
            {
               rc = pShard->syncSend( msg, CATALOG_GROUPID, FALSE, &pAuthRes ) ;
               PD_RC_CHECK( rc, PDERROR, "Session[%s] failed to send auth "
                            "req to catalog, rc=%d", sessionName(), rc ) ;
            }
            if ( NULL == pAuthRes )
            {
               rc = SDB_SYS ;
               PD_LOG( PDERROR, "syncsend return ok but res is NULL" ) ;
               goto error ;
            }
            rc = (( MsgInternalReplyHeader *)pAuthRes)->res ;
            SDB_OSS_FREE( (BYTE*)pAuthRes ) ;
            pAuthRes = NULL ;

            if ( SDB_CLS_NOT_PRIMARY == rc && !hasRetry )
            {
               hasRetry = TRUE ;
               pShard->updateCatGroup( TRUE, CLS_SHARD_TIMEOUT ) ;
               continue ;
            }
            else if ( rc )
            {
               PD_LOG( PDERROR, "Session[%s] auth failed, rc: %d",
                       sessionName(), rc ) ;
               goto error ;
            }
            else
            {
               _authOK = TRUE ;
            }
            break ;
         }
      }

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _pmdLocalSession::_onSysInfoRequest( const CHAR * msg )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN endianConvert = FALSE ;
      MsgSysInfoReply reply ;

      MsgSysInfoReply *pReply = &reply ;
      INT32 replySize = sizeof(reply) ;

      rc = msgExtractSysInfoRequest ( (CHAR*)msg, endianConvert ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to extract sys info "
                    "request, rc = %d", sessionName(), rc ) ;

      // reply
      rc = msgBuildSysInfoReply ( (CHAR**)&pReply, &replySize ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to build sys info reply, "
                    "rc = %d", sessionName(), rc ) ;

      rc = sendMsg ( (const CHAR*)pReply, replySize ) ;
      PD_RC_CHECK ( rc, PDERROR, "Session[%s] failed to send packet, rc = %d",
                    sessionName(), rc ) ;

   done :
      return rc ;
   error :
      disconnect() ;
      goto done ;
   }

   INT32 _pmdLocalSession::_onOPMsg( NET_HANDLE handle, MsgHeader * msg )
   {
      INT32 rc = SDB_OK ;


   done:
      return rc ;
   error:
      goto done ;
   }

}


