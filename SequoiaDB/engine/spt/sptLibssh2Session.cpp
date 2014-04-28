/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sptLibssh2Session.cpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          31/03/2014  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include "sptLibssh2Session.hpp"
#include "pd.hpp"
#include "ossIO.hpp"

using namespace std ;

#define SPT_COPY_BUF_SIZE  8192

namespace engine
{
   _sptLibssh2Session::_sptLibssh2Session( const CHAR *host,
                                           const CHAR *usrname,
                                           const CHAR *passwd)
   :_sptSshSession( host, usrname, passwd ),
    _session( NULL ),
    _channel( NULL )
   {

   }

   _sptLibssh2Session::~_sptLibssh2Session()
   {
      _clearChannel() ;
      _clearSession() ;
   }

   INT32 _sptLibssh2Session::_openSshSession()
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != _sock, "can not be null" )

      _session = libssh2_session_init() ;
      if ( NULL == _session )
      {
         PD_LOG( PDERROR, "failed to init libssh2 session" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = libssh2_session_handshake( _session, _sock->native() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to do handshake with remote:%d", rc ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = libssh2_userauth_password( _session, _usr.c_str(), _passwd.c_str() ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to do handle shake with remote:%d", rc ) ;
         rc = SDB_SYS ;
         goto error ;
      }

   done:
      return rc ;
   error:
      _clearSession() ;
      goto done ;
   }

   INT32 _sptLibssh2Session::exec( const CHAR *cmd,
                                   INT32 &exit,
                                   CHAR *outBuf,
                                   UINT32 len,
                                   UINT32 &read )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != cmd, "can not be null" )
      SDB_ASSERT( NULL != _session, "call open first" )
      SDB_ASSERT( NULL == _channel, "do not share a session in multi threads" )

      string sig ;

      _channel = libssh2_channel_open_session( _session ) ;
      if ( NULL == _channel )
      {
         PD_LOG( PDERROR, "failed to open channel in sesison" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = libssh2_channel_exec( _channel, cmd ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to exec cmd on remote node:%d", rc ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      rc = _read( outBuf, len, read, 0 ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to read output:%d", rc ) ;
         goto error ;
      }

      rc = _done( exit, sig ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "exit of cmd is:%d", exit ) ;
         goto error ;
      }

      if ( SDB_OK != exit )
      {
         rc = SDB_SPT_EVAL_FAIL ;
         PD_LOG( PDERROR, "exit number is:%d", exit ) ;
         if ( 0 == read )
         {
            _read( outBuf, len, read, SSH_EXTENDED_DATA_STDERR ) ;
         }
         goto error ;
      }

   done:
      _clearChannel() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::_read( CHAR *buf,
                                    UINT32 len,
                                    UINT32 &readSize,
                                    INT32 streamId )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != buf &&  0 < len, "impossible" )
      SDB_ASSERT( NULL != _channel, "can not be null" )

      readSize = 0 ;
      if ( libssh2_channel_eof( _channel ) )
      {
         PD_LOG( PDDEBUG, "get eof from the channel" ) ;
         goto done ;
      }

      rc = libssh2_channel_read_ex( _channel, streamId, buf, len ) ;
      if ( 0 == rc )
      {
         PD_LOG( PDDEBUG, "read 0 bytes from channel" ) ;
         goto done ;
      }
      else if ( 0 < rc )
      {
         readSize = rc ;
         rc = SDB_OK ;
         goto done ;
      }
      else
      {
         PD_LOG( PDERROR, "failed to read from channel:%d", rc ) ;
         rc = SDB_SYS ;
         goto error ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::copy2Remote( SPT_CP_PROTOCOL protocol,
                                          const CHAR *local,
                                          const CHAR *dst,
                                          INT32 mode )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != local && NULL != dst, "can not be null" )

      if ( SPT_CP_PROTOCOL_SCP == protocol )
      {
         rc = _scpSend( local, dst, mode ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to scp file:%s, rc:%d", local, rc ) ;
            goto error ;
         }
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "this protocao is not supported yet." ) ;
         goto error ;
      }

   done:
      _clearChannel() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::copyFromRemote( SPT_CP_PROTOCOL protocol,
                                             const CHAR *remote,
                                             const CHAR *local,
                                             INT32 mode )
   {
      INT32 rc = SDB_OK ;
      SDB_ASSERT( NULL != local && NULL != remote, "can not be null" )

      if ( SPT_CP_PROTOCOL_SCP == protocol )
      {
         rc = _scpRecv( remote, local, mode ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to scp_recv file:%s, rc:%d", remote, rc ) ;
            goto error ;
         }
      }
      else
      {
         rc = SDB_INVALIDARG ;
         PD_LOG( PDERROR, "this protocao is not supported yet." ) ;
         goto error ;
      }
   done:
      _clearChannel() ;
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::_done( INT32 &eixtcode, string &exitsignal )
   {
      INT32 rc = SDB_OK ;
      CHAR *sig = NULL ;
      
      if ( NULL != _channel )
      {
         rc = libssh2_channel_close( _channel ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close channel:%d", rc ) ;
            goto error ;
         }

         eixtcode = libssh2_channel_get_exit_status( _channel ) ;

         /// we don't own the signal's mem.
         libssh2_channel_get_exit_signal(_channel, &sig,
                                         NULL, NULL, NULL, NULL, NULL);
         if ( NULL != sig )
         {
            exitsignal.assign( sig ) ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   void _sptLibssh2Session::getLastError( std::string &errMsg )
   {
      CHAR *msg = NULL ;
      INT32 errLen = 0 ;

      /// we do not want to own the mem. set want_buf = 0.
      libssh2_session_last_error( _session, &msg, &errLen, 0 ) ;
      if ( NULL != msg )
      {
         errMsg.assign( msg ) ;
      }

      return ;
   }

   INT32 _sptLibssh2Session::_write2Channel( LIBSSH2_CHANNEL *channel,
                                        const CHAR *buf, SINT64 len )
   {
      INT32 rc = SDB_OK ;
      SINT64 need = len ;
      while ( 0 < need )
      {
         SINT64 once = libssh2_channel_write( channel, buf, len ) ;
         if ( 0 <= once )
         {
            need -= once ;
         }
         else
         {
            PD_LOG( PDERROR, "failed to send data:%lld", once ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::_readFromChannel( LIBSSH2_CHANNEL *channel,
                                               SINT64 len,
                                               CHAR *buf )
   {
      INT32 rc = SDB_OK ;
      SINT64 totalLen = len ;
      SDB_ASSERT( 0 < len, "impossible" )

      while ( 0 < totalLen )
      {
         SINT64 once = libssh2_channel_read( channel, buf, totalLen ) ;
         if ( 0 == once )
         {
            continue ;
         }
         else if ( 0 < once )
         {
            totalLen -= once ;
         }
         else
         {
            PD_LOG( PDERROR, "failed to recv data:%lld", once ) ;
            rc = SDB_SYS ;
            goto error ;
         }
      }
   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::_scpSend( const CHAR *local, const CHAR *dst, INT32 mode )
   {
      INT32 rc = SDB_OK ;
      OSSFILE file ;
      INT64 fileSize = 0 ;

      rc = ossOpen( local, OSS_READONLY, OSS_DEFAULTFILE, file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file: %s, rc: %d", local, rc ) ;
         goto error ;
      }

      rc = ossGetFileSize( &file, &fileSize ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get filesize of file:%s, rc:%d",
                 local, rc ) ;
         goto error ;
      }

      _channel = libssh2_scp_send64( _session, dst, 0755, fileSize, 0, 0 ) ;
      if ( NULL == _channel )
      {
         PD_LOG( PDERROR, "failed to create scp channel" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      {
      CHAR buf[SPT_COPY_BUF_SIZE] ;
      SINT64 readLen = 0 ;
      SINT64 sendLen = 0 ;
      do
      {
         readLen = 0 ;
         rc = ossRead( &file, buf,
                       SPT_COPY_BUF_SIZE, &readLen ) ;
         if ( SDB_OK == rc )
         {
            rc = _write2Channel( _channel, buf, readLen ) ;
            if ( SDB_OK != rc )
            {
               PD_LOG( PDERROR, "failed to write to channel:%d", rc ) ;
               rc = SDB_SYS ;
               goto error ;
            }
            sendLen += readLen ;
         }
         else if ( SDB_EOF == rc )
         {
            rc = SDB_OK ;
            break ;
         }
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to read from file:%d", rc ) ;
            goto error ;
         }
      } while ( TRUE ) ;

      PD_LOG( PDDEBUG, "%lld bytes were written this time", sendLen ) ;

      libssh2_channel_send_eof( _channel ) ;
      
      }
   done:
      if ( file.isOpened() )
      {
         INT32 rc = ossClose( file ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close file:%d", rc ) ;
         }
      }

      return rc ;
   error:
      goto done ;
   }

   INT32 _sptLibssh2Session::_scpRecv( const CHAR *remote,
                                       const CHAR *local,
                                       INT32 mode )
   {
      INT32 rc = SDB_OK ;
      struct stat sb ;
      SINT64 fileSize = 0 ;
      INT32 fMode = OSS_WRITEONLY | OSS_REPLACE ;

      _OSS_FILE file ;
      _channel = libssh2_scp_recv( _session, remote, &sb ) ;
      if ( NULL == _channel )
      {
         PD_LOG( PDERROR, "failed to crate scp recv channel" ) ;
         rc = SDB_SYS ;
         goto error ;
      }

      fileSize = sb.st_size ;

      rc = ossOpen( local,
                    fMode,
                    OSS_DEFAULTFILE, file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to open file:%s, rc:%d", local, rc ) ;
         goto error ;
      }

      {
      CHAR buf[SPT_COPY_BUF_SIZE] ;
      while ( 0 < fileSize )
      {
         SINT64 need = SPT_COPY_BUF_SIZE < fileSize ?
                       SPT_COPY_BUF_SIZE : fileSize ;

         rc = _readFromChannel( _channel, need, buf ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to recv data from channel:%d", rc ) ;
            goto error ;
         }

         rc = _wirte2File( &file, (const CHAR *)buf, need ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         fileSize -= need ;
      }
      }

      rc = ossClose( file ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to close file:%d", rc ) ;
         goto error ;
      }
   done:
      return rc ;
   error:
      if ( file.isOpened() )
      {
         INT32 rc = ossClose( file ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to close file:%d", rc ) ;
         }
         rc = ossDelete( local ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to remove file:%d", rc ) ;
         }
      }
      goto done ;
   }

   void _sptLibssh2Session::_clearChannel()
   {
      if ( NULL != _channel )
      {
         libssh2_channel_close( _channel ) ;
         libssh2_channel_free( _channel ) ;
         _channel = NULL ;
      }
      return ;
   }

   void _sptLibssh2Session::_clearSession()
   {
      if ( NULL != _session )
      {
         libssh2_session_disconnect( _session, "" ) ;
         libssh2_session_free( _session ) ;
         _session = NULL ;
      }

      libssh2_exit() ;

      return ;
   }

   INT32 _sptLibssh2Session::_wirte2File( OSSFILE *file,
                                          const CHAR *buf,
                                          SINT64 len )
   {
      INT32 rc = SDB_OK ;
      SINT64 total = len ;
      while ( 0 < total )
      {
         SINT64 written = 0 ;
         rc = ossWrite( file, buf, total, &written ) ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "failed to write to file:%d", rc ) ;
            goto error ;
         }

         total -= written ;
      }
   done:
      return rc ;
   error:
      goto done ;
   }
}
