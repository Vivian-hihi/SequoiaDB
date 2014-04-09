/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossSocket.hpp

   Descriptive Name = Operating System Services Socket Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains structure for socket object.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSSNETWORK_HPP_
#define OSSNETWORK_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "ossUtil.hpp"
#if defined (_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#else
//#include <winsock2.h>
//#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif
#include <string.h>
#if defined (_WINDOWS)
#define SOCKET_GETLASTERROR   WSAGetLastError()
#define SOCKET_INVALIDSOCKET  INVALID_SOCKET
#define SOCKET_EINTR          WSAEINTR
#define SOCKET_EMFILE         WSAEMFILE
#else
#define SOCKET_GETLASTERROR   errno
#define SOCKET_INVALIDSOCKET  -1
#define SOCKET_EINTR          EINTR
#define SOCKET_EMFILE         EMFILE
#endif
#include "pd.hpp"

// by default 10ms timeout
#define OSS_SOCKET_DFT_TIMEOUT      500

#define OSS_MAX_HOSTNAME            NI_MAXHOST
#define OSS_MAX_SERVICENAME         NI_MAXSERV

// todo: support AF_UNIX later
/*
   _ossSocket define
*/
class _ossSocket : public SDBObject
{
   private :
      SOCKET               _fd ;
      socklen_t            _addressLen ;
      socklen_t            _peerAddressLen ;
      struct sockaddr_in   _sockAddress ;
      struct sockaddr_in   _peerAddress ;

      BOOLEAN              _init ;
      BOOLEAN              _closeWhenDestruct ;
      INT32                _timeout ;

   protected:
      UINT32   _getPort ( sockaddr_in *addr ) ;
      INT32    _getAddress ( sockaddr_in *addr, CHAR *pAddress,
                             UINT32 length ) ;
      INT32    _complete() ;

   public :
      INT32 setSocketLi ( INT32 lOnOff, INT32 linger ) ;

      // Create a listening socket, timeout in millisecond
      _ossSocket ( UINT32 port, INT32 timeoutMilli = 0 ) ;
      // Create a connecting socket, timeout in millisecond
      _ossSocket ( const CHAR *pHostname, UINT32 port, INT32 timeoutMilli = 0 ) ;
      // Create from a existing socket, timeout in millisecond
      _ossSocket ( SOCKET *sock, INT32 timeoutMilli = 0 ) ;

      ~_ossSocket ()
      {
         if ( _closeWhenDestruct )
         {
            close () ;
         }
      }

      inline SOCKET native()const{ return _fd ; }
      inline void closeWhenDestruct( BOOLEAN closeWhenDestruct )
      {
         _closeWhenDestruct = closeWhenDestruct ;
      }

      INT32 initSocket () ;
      INT32 bind_listen () ;
      BOOLEAN isConnected () ;

      INT32 send ( const CHAR *pMsg, INT32 len,
                   INT32 &sentLen,
                   INT32 timeout = OSS_SOCKET_DFT_TIMEOUT,
                   INT32 flags = 0 ) ;
      INT32 recv ( CHAR *pMsg, INT32 len,
                   INT32 &receivedLen,
                   INT32 timeout = OSS_SOCKET_DFT_TIMEOUT,
                   INT32 flags = 0 ) ;
      INT32 recvNF ( CHAR *pMsg, INT32 &len,
                     INT32 timeout = OSS_SOCKET_DFT_TIMEOUT ) ;

      INT32 connect () ;
      void  close () ;
      INT32 accept ( SOCKET *sock, struct sockaddr *addr, socklen_t *addrlen,
                     INT32 timeout = OSS_SOCKET_DFT_TIMEOUT ) ;
      INT32 disableNagle () ;

      UINT32 getPeerPort () ;
      INT32  getPeerAddress ( CHAR *pAddress, UINT32 length ) ;

      UINT32 getLocalPort () ;
      INT32  getLocalAddress ( CHAR *pAddress, UINT32 length ) ;

      INT32 setTimeout ( INT32 milliSeconds ) ;

      static INT32 getHostName ( CHAR *pName, INT32 nameLen ) ;
      static INT32 getPort ( const CHAR *pServiceName, UINT16 &port ) ;
} ;

typedef class _ossSocket ossSocket ;

// define socket functions

INT32    ossInitSocket() ;
void     ossSocketBindListenMutexGet() ;
void     ossSocketBindListenMutexRelease() ;
INT32    ossGetHostName( CHAR *pName, INT32 nameLen ) ;
INT32    ossGetPort( const CHAR *pServiceName, UINT16 &port ) ;

INT32    ossGetAddrInfo( sockaddr_in *addr, CHAR *pAddress, UINT32 length,
                         UINT16 *pPort = NULL ) ;

INT32    ossIP2Str( UINT32 ip, CHAR *pStr, INT32 nameLen ) ;
UINT32   ossStr2IP( const CHAR *pStr ) ;

#endif // OSSNETWORK_HPP_
