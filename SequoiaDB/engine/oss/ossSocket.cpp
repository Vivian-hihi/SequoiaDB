/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossSocket.cpp

   Descriptive Name = Operating System Services Socket

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains functions for socket
   operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "ossSocket.hpp"
#include <stdio.h>
#include "ossLatch.hpp"
#include "pdTrace.hpp"
#include "ossTrace.hpp"

ossSpinXLatch  bindListenLatch ;
// Since this component could be used outside engine, so we don't want to put
// socket initialization in oneTimeInit in pmdMain.cpp
// So we keep a static variable here and whenever the windows socket has been
// initialized, we'll set it to TRUE
#if defined (_WINDOWS)
static BOOLEAN socketInitialized = FALSE ;
#endif

void ossSocketBindListenMutexGet()
{
   bindListenLatch.get() ;
}

void ossSocketBindListenMutexRelease()
{
   bindListenLatch.release() ;
}
// Create a listening socket
// PD_TRACE_DECLARE_FUNCTION ( SDB__OSSSK__OSSSK, "_ossSocket::_ossSocket" )
_ossSocket::_ossSocket ( UINT32 port, INT32 timeoutMilli )
{
   PD_TRACE_ENTRY ( SDB__OSSSK__OSSSK );
   _init = FALSE ;
   _fd = 0 ;
   _timeout = timeoutMilli ;
   _closeWhenDestruct = TRUE ;
   ossMemset ( &_sockAddress, 0, sizeof(sockaddr_in) ) ;
   ossMemset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
   _peerAddressLen = sizeof (_peerAddress) ;
#if defined (_WINDOWS)
   if ( !socketInitialized )
   {
      INT32 rc = SDB_OK ;
      WSADATA data = {0} ;
      rc = WSAStartup ( MAKEWORD ( 2,2 ), &data ) ;
      if ( INVALID_SOCKET == rc )
      {
         // The WSAStartup function directly returns the extended error code in
         // the return value for this function
         // A call to the WSAGetLastError function is not needed and should not
         // be used.
         PD_LOG ( PDERROR, "Failed to startup socket, rc = %d", rc ) ;
      }
      else
         socketInitialized = TRUE ;
   }
#endif
   _sockAddress.sin_family = AF_INET ;
   _sockAddress.sin_addr.s_addr = htonl ( INADDR_ANY ) ;
   _sockAddress.sin_port = htons ( port ) ;
   _addressLen = sizeof ( _sockAddress ) ;
   PD_TRACE_EXIT ( SDB__OSSSK__OSSSK );
}

// Create a connecting socket
// PD_TRACE_DECLARE_FUNCTION ( SDB__OSSSK__OSSSK2, "_ossSocket::_ossSocket" )
_ossSocket::_ossSocket ( const CHAR *pHostname, UINT32 port,
                         INT32 timeoutMilli )
{
   PD_TRACE_ENTRY ( SDB__OSSSK__OSSSK2 );
   struct hostent *hp ;
   _init = FALSE ;
   _closeWhenDestruct = TRUE;
   _timeout = timeoutMilli ;
   _fd = 0 ;
   ossMemset ( &_sockAddress, 0, sizeof(sockaddr_in) ) ;
   ossMemset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
   _peerAddressLen = sizeof (_peerAddress) ;
#if defined (_WINDOWS)
   if ( !socketInitialized )
   {
      INT32 rc = SDB_OK ;
      WSADATA data = {0} ;
      rc = WSAStartup ( MAKEWORD ( 2,2 ), &data ) ;
      if ( INVALID_SOCKET == rc )
      {
         // The WSAStartup function directly returns the extended error code in
         // the return value for this function
         // A call to the WSAGetLastError function is not needed and should not
         // be used.
         PD_LOG ( PDERROR, "Failed to startup socket, rc = %d", rc ) ;
      }
      else
         socketInitialized = TRUE ;
   }
#endif
   _sockAddress.sin_family = AF_INET ;
#if defined (_WINDOWS)
   if ( (hp = gethostbyname ( pHostname )))
#elif defined (_LINUX)
   struct hostent hent ;
   struct hostent *retval = NULL ;
   INT32 error            = 0 ;
   CHAR hbuf[8192]        = {0} ;
   hp                     = &hent ;

   if ( (0 == gethostbyname_r ( pHostname, &hent, hbuf, sizeof(hbuf),
                                &retval, &error )) &&
         NULL != retval )
#endif
   {
      UINT32 *pAddr = (UINT32 *)hp->h_addr_list[0] ;
      if ( pAddr )
         _sockAddress.sin_addr.s_addr = *pAddr ;
   }
   else
      _sockAddress.sin_addr.s_addr = inet_addr ( pHostname ) ;
   _sockAddress.sin_port = htons ( port ) ;
   _addressLen = sizeof ( _sockAddress ) ;
   PD_TRACE_EXIT ( SDB__OSSSK__OSSSK2 );
}
// Create from a existing socket
// PD_TRACE_DECLARE_FUNCTION ( SDB__OSSSK__OSSSK3, "_ossSocket::_ossSocket" )
_ossSocket::_ossSocket ( SOCKET *sock, INT32 timeoutMilli )
{
   SDB_ASSERT ( sock, "Input sock is NULL" ) ;
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__OSSSK__OSSSK3 );
   _fd = *sock ;
   _init = TRUE ;
   _timeout = timeoutMilli ;
   _closeWhenDestruct = TRUE ;
   _addressLen = sizeof ( _sockAddress ) ;

   ossMemset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
   _peerAddressLen = sizeof ( _peerAddress ) ;

#if defined (_WINDOWS)
   if ( !socketInitialized )
   {
      INT32 rc = SDB_OK ;
      WSADATA data = {0} ;
      rc = WSAStartup ( MAKEWORD ( 2,2 ), &data ) ;
      if ( INVALID_SOCKET == rc )
      {
         // The WSAStartup function directly returns the extended error code in
         // the return value for this function
         // A call to the WSAGetLastError function is not needed and should not
         // be used.
         PD_LOG ( PDERROR, "Failed to startup socket, rc = %d", rc ) ;
      }
      else
         socketInitialized = TRUE ;
   }
#endif
   rc = getsockname ( _fd, (sockaddr*)&_sockAddress, &_addressLen ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to getsockname from socket %d", _fd ) ;
      _init = FALSE ;
   }
   else
   {
      //get peer address
      rc = getpeername ( _fd, (sockaddr*)&_peerAddress, &_peerAddressLen ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to getpeername from socket %d", _fd ) ;
      }
   }
   setTimeout ( _timeout ) ;
   PD_TRACE_EXIT ( SDB__OSSSK__OSSSK3 );
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_INITTSK, "ossSocket::initSocket" )
INT32 ossSocket::initSocket ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_INITTSK );
   if ( _init )
   {
      goto done ;
   }
   ossMemset ( &_peerAddress, 0, sizeof(sockaddr_in) ) ;
   _peerAddressLen = sizeof ( _peerAddress ) ;

   _fd = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ;
   if ( -1 == _fd )
   {
      PD_LOG ( PDERROR, "Failed to initialize socket, error = %d",
               SOCKET_GETLASTERROR ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
   _init = TRUE ;
   // settimeout should always return SDB_OK
   setTimeout ( _timeout ) ;
done :
   PD_TRACE_EXITRC ( SDB_OSSSK_INITTSK, rc );
   return rc ;
error:
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_SETSKLI, "ossSocket::setSocketLi" )
INT32 ossSocket::setSocketLi ( INT32 lOnOff, INT32 linger )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_SETSKLI );
   SDB_ASSERT ( _init, "socket is not initialized" )
   struct linger _linger ;
   _linger.l_onoff = lOnOff ;
   _linger.l_linger = linger ;
   rc = setsockopt ( _fd, SOL_SOCKET, SO_LINGER,
                     (const char*)&_linger, sizeof (_linger) ) ;

   PD_TRACE_EXITRC ( SDB_OSSSK_SETSKLI, rc );
   return rc ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_BIND_LSTN, "ossSocket::bind_listen" )
INT32 ossSocket::bind_listen ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_BIND_LSTN );
   INT32 temp = 1 ;
   SDB_ASSERT ( _init, "socket is not initialized" )
   // Allows the socket to be bound to an address that is already in use.
   // For database shutdown and restart right away, before socket close
   rc = setsockopt ( _fd, SOL_SOCKET,
                     SO_REUSEADDR,
                     (char*)&temp,
                     sizeof (INT32) );
   if ( rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt SO_REUSEADDR, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }
   rc = setSocketLi( 1, 30 ) ;
   if ( rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt SO_LINGER, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }
   rc = ::bind ( _fd, (struct sockaddr *)&_sockAddress, _addressLen ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to bind socket, rc = %d",
               SOCKET_GETLASTERROR ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }

   rc = listen ( _fd, SOMAXCONN ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to listen socket, rc = %d",
               SOCKET_GETLASTERROR ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB_OSSSK_BIND_LSTN, rc );
   return rc ;
error :
   close () ;
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_SEND, "ossSocket::send" )
INT32 ossSocket::send ( const CHAR *pMsg, INT32 len,
                        INT32 &sentLen,
                        INT32 timeout, INT32 flags )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_SEND );
   SDB_ASSERT ( pMsg, "message is NULL" )
   SDB_ASSERT ( _init, "socket is not initialized" )
   sentLen = 0 ;
   SOCKET maxFD = _fd ;
   struct timeval maxSelectTime ;
   fd_set fds ;

   maxSelectTime.tv_sec = timeout / 1000 ;
   maxSelectTime.tv_usec = ( timeout % 1000 ) * 1000 ;
   // if we don't expect to receive anything, no need to continue
   if ( 0 == len )
      return SDB_OK ;
   // wait loop until the socket is ready
   while ( TRUE )
   {
      FD_ZERO ( &fds ) ;
      FD_SET ( _fd, &fds ) ;
      rc = select ( maxFD + 1, NULL, &fds, NULL,
                    timeout>=0?&maxSelectTime:NULL ) ;

      // 0 means timeout
      if ( 0 == rc )
      {
         rc = SDB_TIMEOUT ;
         goto done ;
      }
      // if < 0, means something wrong
      if ( 0 > rc )
      {
         rc = SOCKET_GETLASTERROR ;
         // if we failed due to interrupt, let's continue
         if (
#if defined (_WINDOWS)
               WSAEINTR
#else
               EINTR
#endif
               == rc )
         {
            continue ;
         }
         PD_LOG ( PDERROR, "Failed to select from socket, rc = %d", rc) ;
         rc = SDB_NETWORK ;
         goto error ;
      }

      // if the socket we interested is not receiving anything, let's continue
      if ( FD_ISSET ( _fd, &fds ) )
      {
         break ;
      }
   }
   while ( len > 0 )
   {

#if defined (_WINDOWS)
      rc = ::send ( _fd, pMsg, len, flags ) ;
      if ( SOCKET_ERROR == rc )
#else
      // MSG_NOSIGNAL : Requests not to send SIGPIPE on errors on stream
      // oriented sockets when the other end breaks the connection. The EPIPE
      // error is still returned.
      rc = ::send ( _fd, pMsg, len, MSG_NOSIGNAL|flags ) ;
      if ( -1 == rc )
#endif
      {
         rc = SOCKET_GETLASTERROR ;
#if defined (_WINDOWS)
         if ( WSAETIMEDOUT == rc && _timeout > 0 )
#else
         if ( (EAGAIN == rc || EWOULDBLOCK == rc) &&
              _timeout > 0 )
#endif
         {
            rc = SDB_TIMEOUT ;
            goto error ;
         }
         PD_LOG ( PDERROR, "Failed to send, rc = %d", SOCKET_GETLASTERROR ) ;
         rc = SDB_NETWORK ;
         goto error ;
      }
      sentLen += rc ;
      len -= rc ;
      pMsg += rc ;
   }
   rc = SDB_OK ;
done :
   PD_TRACE_EXITRC ( SDB_OSSSK_SEND, rc );
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_ISCONN, "ossSocket::isConnected" )
BOOLEAN ossSocket::isConnected ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_ISCONN );
#if defined (_WINDOWS)
   rc = ::send ( _fd, "", 0, 0 ) ;
   if ( SOCKET_ERROR == rc )
#else
   // MSG_NOSIGNAL : Requests not to send SIGPIPE on errors on stream
   // oriented sockets when the other end breaks the connection. The EPIPE
   // error is still returned.
   //rc = ::send ( _fd, "", 0, MSG_NOSIGNAL ) ;
   //if ( 0 > rc )
   rc = ::recv ( _fd, NULL, 0, MSG_DONTWAIT ) ;
   if ( 0 == rc )
#endif
   {
      PD_TRACE_EXIT ( SDB_OSSSK_ISCONN );
      return FALSE ;
   }
   PD_TRACE_EXIT ( SDB_OSSSK_ISCONN );
   return TRUE ;
}

#define MAX_RECV_RETRIES 5
INT32 ossSocket::recv ( CHAR *pMsg, INT32 len,
                        INT32 &receivedLen,
                        INT32 timeout, INT32 flags )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( _init, "socket is not initialized" )
   SDB_ASSERT ( pMsg, "message is NULL" )
   UINT32 retries = 0 ;
   SOCKET maxFD = _fd ;
   struct timeval maxSelectTime ;
   fd_set fds ;
   receivedLen = 0 ;
   // if we don't expect to receive anything, no need to continue
   if ( 0 == len )
      return SDB_OK ;

   maxSelectTime.tv_sec = timeout / 1000 ;
   maxSelectTime.tv_usec = ( timeout % 1000 ) * 1000 ;
   // wait loop until either we timeout or get a message
   while ( true )
   {
      FD_ZERO ( &fds ) ;
      FD_SET ( _fd, &fds ) ;
      rc = select ( maxFD + 1, &fds, NULL, NULL,
                    timeout>=0?&maxSelectTime:NULL ) ;

      // 0 means timeout
      if ( 0 == rc )
      {
         rc = SDB_TIMEOUT ;
         goto done ;
      }
      // if < 0, means something wrong
      if ( 0 > rc )
      {
         rc = SOCKET_GETLASTERROR ;
         // if we failed due to interrupt, let's continue
         if (
#if defined (_WINDOWS)
               WSAEINTR
#else
               EINTR
#endif
               == rc )
         {
            continue ;
         }
         PD_LOG ( PDERROR, "Failed to select from socket, rc = %d", rc) ;
         rc = SDB_NETWORK ;
         goto error ;
      }

      // if the socket we interested is not receiving anything, let's continue
      if ( FD_ISSET ( _fd, &fds ) )
      {
         break ;
      }
   }
   // Once we start receiving message, there's no chance to timeout, in order to
   // prevent partial read
   while ( len > 0 )
   {
#if defined (_WINDOWS)
      rc = ::recv ( _fd, pMsg, len, flags ) ;
#else
      // MSG_NOSIGNAL : Requests not to send SIGPIPE on errors on stream
      // oriented sockets when the other end breaks the connection. The EPIPE
      // error is still returned.
      rc = ::recv ( _fd, pMsg, len, MSG_NOSIGNAL|flags ) ;
#endif
      if ( rc > 0 )
      {
         if ( flags & MSG_PEEK )
         {
            goto done ;
         }
         receivedLen += rc ;
         len -= rc ;
         pMsg += rc ;
      }
      else if ( rc == 0 )
      {
         PD_LOG ( PDERROR, "Peer unexpected shutdown" ) ;
         rc = SDB_NETWORK_CLOSE ;
         goto error ;
      }
      else
      {
         // if rc < 0
         rc = SOCKET_GETLASTERROR ;
#if defined (_WINDOWS)
         if ( WSAETIMEDOUT == rc && _timeout > 0 )
#else
         if ( (EAGAIN == rc || EWOULDBLOCK == rc) &&
              _timeout > 0 )
#endif
         {
            rc = SDB_TIMEOUT ;
            goto error ;
         }
         if ( (
#if defined (_WINDOWS)
               WSAEINTR
#else
               EINTR
#endif
               == rc ) && ( retries < MAX_RECV_RETRIES ) )
         {
            // less than max_recv_retries number, let's retry
            retries ++ ;
            continue ;
         }
         // something bad when get here
         PD_LOG ( PDERROR, "Recv() Failed: rc = %d", rc ) ;
         rc = SDB_NETWORK ;
         goto error ;
      }
   }
   // Everything is fine when get here
   rc = SDB_OK ;
done :
   return rc ;
error :
   goto done ;
}

INT32 ossSocket::recvNF ( CHAR *pMsg, INT32 &len,
                          INT32 timeout )
{
   INT32 rc = SDB_OK ;
   SDB_ASSERT ( _init, "socket is not initialized" )
   SDB_ASSERT ( pMsg, "message is NULL" )
   UINT32 retries = 0 ;
   SOCKET maxFD = _fd ;
   struct timeval maxSelectTime ;
   fd_set fds ;
   // if we don't expect to receive anything, no need to continue
   if ( 0 == len )
      return SDB_OK ;

   maxSelectTime.tv_sec = timeout / 1000 ;
   maxSelectTime.tv_usec = ( timeout % 1000 ) * 1000 ;
   // wait loop until either we timeout or get a message
   while ( true )
   {
      FD_ZERO ( &fds ) ;
      FD_SET ( _fd, &fds ) ;
      rc = select ( maxFD + 1, &fds, NULL, NULL,
                    timeout>=0?&maxSelectTime:NULL ) ;

      // 0 means timeout
      if ( 0 == rc )
      {
         rc = SDB_TIMEOUT ;
         goto done ;
      }
      // if < 0, means something wrong
      if ( 0 > rc )
      {
         rc = SOCKET_GETLASTERROR ;
         // if we failed due to interrupt, let's continue
         if (
#if defined (_WINDOWS)
               WSAEINTR
#else
               EINTR
#endif
               == rc )
         {
            continue ;
         }
         PD_LOG ( PDERROR, "Failed to select from socket, rc = %d", rc ) ;
         rc = SDB_NETWORK ;
         goto error ;
      }

      // if the socket we interested is not receiving anything, let's continue
      if ( FD_ISSET ( _fd, &fds ) )
      {
         break ;
      }
   }

#if defined (_WINDOWS)
   rc = ::recv ( _fd, pMsg, len, 0 ) ;
#else
   // MSG_NOSIGNAL : Requests not to send SIGPIPE on errors on stream
   // oriented sockets when the other end breaks the connection. The EPIPE
   // error is still returned.
   rc = ::recv ( _fd, pMsg, len, MSG_NOSIGNAL ) ;
#endif
   if ( rc > 0 )
   {
      len = rc ;
   }
   else if ( rc == 0 )
   {
      PD_LOG ( PDERROR, "Peer unexpected shutdown" ) ;
      rc = SDB_NETWORK_CLOSE ;
      goto error ;
   }
   else
   {
      // if rc < 0
      rc = SOCKET_GETLASTERROR ;
#if defined (_WINDOWS)
      if ( WSAETIMEDOUT == rc && _timeout > 0 )
#else
      if ( (EAGAIN == rc || EWOULDBLOCK == rc) &&
           _timeout > 0 )
#endif
      {
         // if we timeout, it's partial message and we should restart
         PD_LOG ( PDWARNING, "Recv() timeout: rc = %d", rc ) ;
         rc = SDB_TIMEOUT ;
         goto error ;
      }
      if ( (
#if defined (_WINDOWS)
            WSAEINTR
#else
            EINTR
#endif
            == rc ) && ( retries < MAX_RECV_RETRIES ) )
      {
         // less than max_recv_retries number, let's retry
         retries ++ ;
      }
      // something bad when get here
      PD_LOG ( PDERROR, "Recv() Failed: rc = %d", rc ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
   // Everything is fine when get here
   rc = SDB_OK ;
done :
   return rc ;
error :
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_CONNECT, "ossSocket::connect" )
INT32 ossSocket::connect ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_CONNECT );
   SDB_ASSERT ( _init, "socket is not initialized" )
   SDB_ASSERT ( !_peerAddress.sin_addr.s_addr,
                "Cannot connect without close/init" )

#if defined (_LINUX)

   INT32 flags = fcntl( native(), F_GETFL, 0) ;
   if ( fcntl( native(), F_SETFL, flags | O_NONBLOCK ) <0 )
   {
      PD_LOG( PDERROR, "failed to fcntl sock:%d",native() ) ;
      rc = SDB_SYS ;
      goto error ;
   }

   rc = ::connect ( _fd, (struct sockaddr *) &_sockAddress, _addressLen ) ;
   if ( rc != SDB_OK )
   {
      if ( SOCKET_GETLASTERROR == EINPROGRESS )
      {
         rc = _complete() ;
         if ( SDB_OK != rc )
         {
            PD_LOG( PDERROR, "Failed to complete connect, rc = %d", rc ) ;
            goto error ;
         }
      }
      else
      {
         PD_LOG ( PDERROR, "Failed to connect, rc = %d", SOCKET_GETLASTERROR ) ;
         rc = SDB_NETWORK ;
         goto error ;
      }
   }
   else
   {
      /// do nothing.
   }

   if ( fcntl( native(), F_SETFL, flags & ~O_NONBLOCK ) <0 )
   {
      PD_LOG( PDERROR, "failed to fcntl sock:%d",native() ) ;
      close() ;
      rc = SDB_SYS ;
      goto error ;
   }
#elif defined (_WINDOWS)

   rc = ::connect ( _fd, (struct sockaddr *) &_sockAddress, _addressLen ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to connect, rc = %d", SOCKET_GETLASTERROR ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
#endif

   //get local address
   rc = getsockname ( _fd, (sockaddr*)&_sockAddress, &_addressLen ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get local address, rc=%d", rc ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
   //get peer address
   rc = getpeername ( _fd, (sockaddr*)&_peerAddress, &_peerAddressLen ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get peer address, rc=%d", rc ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
   // if the local addr is the same with remote addr
   if ( _sockAddress.sin_port == _peerAddress.sin_port &&
        _sockAddress.sin_addr.s_addr == _peerAddress.sin_addr.s_addr )
   {
      PD_LOG( PDERROR, "Local addr is the same with remote addr, "
              "local port: %u, remote port: %u", getLocalPort(),
              getPeerPort() ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }

done :
   PD_TRACE_EXITRC ( SDB_OSSSK_CONNECT, rc );
   return rc ;
error :
   close() ;
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_CLOSE, "ossSocket::close" )
void ossSocket::close ()
{
   PD_TRACE_ENTRY ( SDB_OSSSK_CLOSE );
   if ( _init )
   {
#if defined (_WINDOWS)
      closesocket ( _fd ) ;
#else
      INT32 i = 0 ;
      i = ::close ( _fd ) ;
      if ( i < 0 )
      {
         i = -1 ;
      }
#endif
      _init = FALSE ;
   }
   PD_TRACE_EXIT ( SDB_OSSSK_CLOSE );
}

INT32 ossSocket::accept ( SOCKET *sock, struct sockaddr *addr, socklen_t
                          *addrlen, INT32 timeout )
{
   INT32 rc = SDB_OK ;
   SOCKET maxFD = _fd ;
   INT32 sysError = 0 ;
   INT32 tmpErr = 0 ;
   struct timeval maxSelectTime ;
   SDB_ASSERT ( _init, "socket is not initialized" )
   SDB_ASSERT ( sock, "Output sock is NULL" )

   fd_set fds ;
   maxSelectTime.tv_sec = timeout / 1000 ;
   maxSelectTime.tv_usec = ( timeout % 1000 ) * 1000 ;
   while ( true )
   {
      FD_ZERO ( &fds ) ;
      FD_SET ( _fd, &fds ) ;
      rc = select ( maxFD + 1, &fds, NULL, NULL,
                    timeout>=0?&maxSelectTime:NULL ) ;

      // 0 means timeout
      if ( 0 == rc )
      {
         *sock = 0 ;
         rc = SDB_TIMEOUT ;
         goto done ;
      }
      // if < 0, means something wrong
      if ( 0 > rc )
      {
         sysError = SOCKET_GETLASTERROR ;
         // if we failed due to interrupt, let's continue
         if (
#if defined (_WINDOWS)
               WSAEINTR
#else
               EINTR
#endif
               == sysError )
         {
            continue ;
         }
         PD_LOG ( PDERROR, "Failed to select from socket, rc = %d",
                  sysError );
         rc = SDB_NETWORK ;
         goto error ;
      }

      // if the socket we interested is not receiving anything, let's continue
      if ( FD_ISSET ( _fd, &fds ) )
      {
         break ;
      }
   }
   // reset rc back to SDB_OK, since the rc now is the result from select()
   rc = SDB_OK ;
   *sock = ::accept ( _fd, addr, addrlen ) ;
#if defined (_WINDOWS)
   tmpErr = WSAEMFILE ;
   if ( INVALID_SOCKET == *sock )
#else
   tmpErr = EMFILE ;
   if ( -1 == *sock )
#endif
   {
      sysError = SOCKET_GETLASTERROR ;
      rc = ( tmpErr == sysError ) ? SDB_TOO_MANY_OPEN_FD : SDB_NETWORK ;
      PD_LOG ( ( rc == SDB_NETWORK ? PDERROR : PDINFO ) ,
               "Failed to accept socket, rc = %d", sysError ) ;
      goto error ;
   }
done :
   return rc ;
error :
   if ( rc != SDB_TOO_MANY_OPEN_FD )
   {
      close () ;
   }
   goto done ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_DISNAG, "ossSocket::disableNagle" )
INT32 ossSocket::disableNagle ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_DISNAG );
   INT32 temp = 1 ;
   SDB_ASSERT ( _init, "socket is not initialized" )
   rc = setsockopt ( _fd, IPPROTO_TCP, TCP_NODELAY, (CHAR *) &temp,
                     sizeof ( INT32 ) ) ;
   if ( rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }

   rc = setsockopt ( _fd, SOL_SOCKET, SO_KEEPALIVE, (CHAR *) &temp,
                     sizeof ( INT32 ) ) ;
   if ( rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }
   PD_TRACE_EXITRC ( SDB_OSSSK_DISNAG, rc );
   return rc ;
}

UINT32 ossSocket::_getPort ( sockaddr_in *addr )
{
   SDB_ASSERT ( _init, "socket is not initialized" )
   return ntohs ( addr->sin_port ) ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK__GETADDR, "ossSocket::_getAddress" )
INT32 ossSocket::_getAddress ( sockaddr_in *addr, CHAR *pAddress, UINT32 length )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK__GETADDR );
   SDB_ASSERT ( _init, "socket is not initialized" )
   length = length < NI_MAXHOST ? length : NI_MAXHOST ;
   rc = getnameinfo ( (struct sockaddr *)addr, sizeof(sockaddr), pAddress, length,
                       NULL, 0, NI_NUMERICHOST ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to getnameinfo, rc = %d",
               SOCKET_GETLASTERROR ) ;
      rc = SDB_NETWORK ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB_OSSSK__GETADDR, rc );
   return rc ;
error :
   goto done ;
}

UINT32 ossSocket::getLocalPort ()
{
   return _getPort ( &_sockAddress ) ;
}

UINT32 ossSocket::getPeerPort ()
{
   return _getPort ( &_peerAddress ) ;
}

INT32 ossSocket::getLocalAddress ( CHAR * pAddress, UINT32 length )
{
   return _getAddress ( &_sockAddress, pAddress, length ) ;
}

INT32 ossSocket::getPeerAddress ( CHAR * pAddress, UINT32 length )
{
   return _getAddress ( &_peerAddress, pAddress, length ) ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB_OSSSK_SETTMOUT, "ossSocket::setTimeout" )
INT32 ossSocket::setTimeout ( INT32 milliSeconds )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_OSSSK_SETTMOUT );
   SDB_ASSERT ( _init, "socket is not initialized" )
   struct timeval tv ;
   tv.tv_sec = milliSeconds / 1000 ;
   tv.tv_usec = ( milliSeconds % 1000 ) * 1000 ;
   // windows take milliseconds as parameter
   // but linux takes timeval as input
#if defined (_WINDOWS)
   // convert microseconds to milliseconds in DWORD
   tv.tv_sec = milliSeconds ;
   rc = setsockopt ( _fd, SOL_SOCKET, SO_RCVTIMEO, ( char* ) &tv.tv_sec,
                     sizeof ( INT32 ) ) ;
   if ( SOCKET_ERROR == rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }

   rc = setsockopt ( _fd, SOL_SOCKET, SO_SNDTIMEO, ( char* ) &tv.tv_sec,
                     sizeof ( INT32 ) ) ;
   if ( SOCKET_ERROR == rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }
#else
   rc = setsockopt ( _fd, SOL_SOCKET, SO_RCVTIMEO, ( char* ) &tv,
                     sizeof ( tv ) ) ;
   if ( rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }

   rc = setsockopt ( _fd, SOL_SOCKET, SO_SNDTIMEO, ( char* ) &tv,
                     sizeof ( tv ) ) ;
   if ( rc )
   {
      PD_LOG ( PDWARNING, "Failed to setsockopt, rc = %d",
               SOCKET_GETLASTERROR ) ;
   }
#endif
   PD_TRACE_EXITRC ( SDB_OSSSK_SETTMOUT, rc );
   return rc ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__OSSSK_GETHNM, "_ossSocket::getHostName" )
INT32 _ossSocket::getHostName ( CHAR *pName, INT32 nameLen )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__OSSSK_GETHNM );
#if defined (_WINDOWS)
   if ( !socketInitialized )
   {
      INT32 rc = SDB_OK ;
      WSADATA data = {0} ;
      rc = WSAStartup ( MAKEWORD ( 2,2 ), &data ) ;
      if ( INVALID_SOCKET == rc )
      {
         PD_LOG ( PDERROR, "Failed to startup socket, rc = %d", rc ) ;
         rc = SDB_NETWORK ;
         goto done ;
      }
      else
         socketInitialized = TRUE ;
   }
#endif
   rc = gethostname ( pName, nameLen ) ;
#if defined (_WINDOWS)
done :
#endif
   PD_TRACE_EXITRC ( SDB__OSSSK_GETHNM, rc );
   return rc ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__OSSSK_GETPORT, "_ossSocket::getPort" )
INT32 _ossSocket::getPort ( const CHAR *pServiceName, UINT16 &port )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB__OSSSK_GETPORT );
#if defined (_WINDOWS)
   if ( !socketInitialized )
   {
      INT32 rc = SDB_OK ;
      WSADATA data = {0} ;
      rc = WSAStartup ( MAKEWORD ( 2,2 ), &data ) ;
      if ( INVALID_SOCKET == rc )
      {
         PD_LOG ( PDERROR, "Failed to startup socket, rc = %d", rc ) ;
         rc = SDB_NETWORK ;
         goto done ;
      }
      else
         socketInitialized = TRUE ;
   }
#endif
   {
      struct servent *servinfo ;
      servinfo = getservbyname ( pServiceName, "tcp" ) ;
      if ( !servinfo )
         port = atoi ( pServiceName ) ;
      else
         port = (UINT16)ntohs(servinfo->s_port) ;
   }
#if defined (_WINDOWS)
done :
#endif
   PD_TRACE_EXITRC ( SDB__OSSSK_GETPORT, rc );
   return rc ;
}

// PD_TRACE_DECLARE_FUNCTION ( SDB__OSSSK__COMPLETE, "_ossSocket::_complete" )
INT32 _ossSocket::_complete()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY( SDB__OSSSK__COMPLETE ) ;
#if defined (_LINUX)
   INT32 err = 0 ;
   socklen_t errlen = sizeof(err) ;
   timeval tv ;
   tv.tv_sec = 0 ;
   tv.tv_usec = 100000 ; // 100 ms
   fd_set wfd ;
   FD_ZERO( &wfd ) ;
   FD_SET( _fd, &wfd ) ;
   if ( -1 == ::select( _fd + 1, NULL, &wfd, NULL, &tv ) )
   {
      PD_LOG( PDERROR, "select(2) error: %d(%s)", errno, strerror(errno) ) ;
      rc = SDB_SYS ;
      goto error ;
   }

   if ( !FD_ISSET( _fd, &wfd ) )
   {
      errno = ETIMEDOUT ;
      PD_LOG( PDERROR, "connect timeout" ) ;
      rc = SDB_TIMEOUT ;
      goto error ;
   }

   if ( ::getsockopt( _fd, SOL_SOCKET, SO_ERROR,
                      &err, &errlen ) < 0 )
   {
      PD_LOG( PDERROR, "failed to getsockopt" ) ;
      rc = SDB_SYS ;
      goto error ;
   }

   if ( SDB_OK != err )
   {
      errno = err ;
      PD_LOG( PDERROR, "failed to connect to remote: %d", err ) ;
      rc = SDB_NET_CANNOT_CONNECT ;
      goto error ;
   }
#endif
done:
   PD_TRACE_EXITRC( SDB__OSSSK__COMPLETE, rc ) ;
   return rc ;
error:
   close() ;
   goto done ;
}

