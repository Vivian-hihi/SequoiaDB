/******************************************************************************
   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netEventHandler.cpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  YW  Initial Draft

   Last Changed =

*******************************************************************************/

#include <boost/bind.hpp>

#include "core.hpp"
#include "netEventHandler.hpp"
#include "netFrame.hpp"
#include "ossMem.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "netTrace.hpp"

using namespace boost::asio::ip ;
namespace engine
{
   const UINT32 NET_MSG_MAX_LEN = 1024 * 1024 * 512 ;

   // PD_TRACE_DECLARE_FUNCTION ( SDB_REMOTEADDR, "remoteAddr" )
   static string remoteAddr( const tcp::socket &sock )
   {
      PD_TRACE_ENTRY ( SDB_REMOTEADDR );
      string addr ;
      try
      {
         addr = sock.remote_endpoint().address().to_string() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "get remote end point err: %s",
                 e.what() ) ;
      }
      PD_TRACE_EXIT ( SDB_REMOTEADDR );
      return addr ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB_REMOTEPORT, "remotePort" )
   static UINT32 remotePort( const tcp::socket &sock )
   {
      PD_TRACE_ENTRY ( SDB_REMOTEPORT );
      UINT32 port = 0 ;
      try
      {
         port = sock.remote_endpoint().port() ;
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "get remote end point err: %s",
                 e.what() ) ;
      }
      PD_TRACE1 ( SDB_REMOTEPORT, PD_PACK_UINT(port) );
      PD_TRACE_EXIT ( SDB_REMOTEPORT );
      return port ;
   }

   _netEventHandler::_netEventHandler( _netFrame *frame ):
                                      _sock(frame->ioservice()),
                                      _buf(NULL),
                                      _bufLen(0),
                                      _state(NET_EVENT_HANDLER_STATE_HEADER),
                                      _frame(frame),
                                      _handle(_frame->allocateHandle())
   {
      _id.value      = MSG_INVALID_ROUTEID ;
      _isConnected   = FALSE ;
      _isInAsync     = FALSE ;
   }


   _netEventHandler::~_netEventHandler()
   {
      close() ;
      if ( NULL != _buf )
      {
         SDB_OSS_FREE( _buf ) ;
      }
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETEVNHND_SETOPT, "_netEventHandler::setOpt" )
   void _netEventHandler::setOpt()
   {
      PD_TRACE_ENTRY ( SDB__NETEVNHND_SETOPT );

      _isConnected = TRUE ;

      try
      {
         _sock.set_option( tcp::no_delay(TRUE) ) ;
         _sock.set_option( tcp::socket::keep_alive(TRUE) ) ;
#if defined (_LINUX)
         INT32 keepAlive = 1 ;
         INT32 keepIdle = 15 ;
         INT32 keepInterval = 5 ;
         INT32 keepCount = 3 ;
         INT32 res = SDB_OK ;
         struct timeval sendtimeout ;
         sendtimeout.tv_sec = 1 ;
         sendtimeout.tv_usec = 0 ;
         SOCKET nativeSock = _sock.native() ;
         /// duplicate set?
         res = setsockopt( nativeSock, SOL_SOCKET, SO_KEEPALIVE,
                     ( void *)&keepAlive, sizeof(keepAlive) ) ;
         if ( SDB_OK != res )
         {
            PD_LOG( PDERROR, "failed to set keepalive of sock[%d],"
                    "err:%d", nativeSock, res ) ;
         }
         res = setsockopt( nativeSock, SOL_TCP, TCP_KEEPIDLE,
                     ( void *)&keepIdle, sizeof(keepIdle) ) ;
         if ( SDB_OK != res )
         {
            PD_LOG( PDERROR, "failed to set keepidle of sock[%d],"
                    "err:%d", nativeSock, res ) ;
         }
         res = setsockopt( nativeSock, SOL_TCP, TCP_KEEPINTVL,
                     ( void *)&keepInterval, sizeof(keepInterval) ) ;
         if ( SDB_OK != res )
         {
            PD_LOG( PDERROR, "failed to set keepintvl of sock[%d],"
                    "err:%d", nativeSock, res ) ;
         }
         res = setsockopt( nativeSock, SOL_TCP, TCP_KEEPCNT,
                     ( void *)&keepCount, sizeof(keepCount) ) ;
         if ( SDB_OK != res )
         {
            PD_LOG( PDERROR, "failed to set keepcnt of sock[%d],"
                    "err:%d", nativeSock, res ) ;
         }
         res = setsockopt( nativeSock, SOL_SOCKET, SO_SNDTIMEO,
                           ( CHAR * )&sendtimeout, sizeof(struct timeval) ) ;
         if ( SDB_OK != res )
         {
            PD_LOG( PDERROR, "failed to set sndtimeout of sock[%d],"
                    "err:%d", nativeSock, res ) ;
         }
#endif
      }
      catch( std::exception &e )
      {
         PD_LOG( PDERROR, "failed to set no delay:%s", e.what() ) ;
      }
      PD_TRACE_EXIT ( SDB__NETEVNHND_SETOPT );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETEVNHND_SYNCCONN, "_netEventHandler::syncConnect" )
   INT32 _netEventHandler::syncConnect( const CHAR *hostName,
                                        const CHAR *serviceName )
   {
      SDB_ASSERT( NULL != hostName, "hostName should not be NULL" )
      SDB_ASSERT( NULL != serviceName, "serviceName should not be NULL" )

      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETEVNHND_SYNCCONN );

      if ( _isConnected )
      {
         close() ;
      }

/*
      try
      {

         boost::system::error_code ec ;
         tcp::resolver::query query ( tcp::v4(), hostName, serviceName ) ;
         tcp::resolver resolver ( _frame->ioservice() ) ;
         tcp::resolver::iterator itr = resolver.resolve ( query ) ;
         ip::tcp::endpoint endpoint = *itr ;
         _sock.open( tcp::v4()) ;

         _sock.connect( endpoint, ec ) ;
         /// may return ok when we in a local area network.
         if ( ec )
         {
            if ( boost::asio::error::would_block ==
                 ec )
            {
            rc = _complete( _sock.native() ) ;
            if ( SDB_OK != rc )
            {
               _sock.close() ;
               PD_LOG ( PDWARNING,
                  "Failed to connect to %s: %s: timeout",
                  hostName, serviceName ) ;
               goto error ;
            }
            }
            else
            {
               PD_LOG ( PDWARNING,
                  "Failed to connect to %s: %s: %s", hostName, serviceName,
                  ec.message().c_str()) ;
               rc = SDB_NET_CANNOT_CONNECT ;
               _sock.close() ;
               goto error ;
            }
         }
      }
      catch ( boost::system::system_error &e )
      {
         PD_LOG ( PDWARNING,
                  "Failed to connect to %s: %s: %s", hostName, serviceName,
                  e.what() ) ;
         rc = SDB_NET_CANNOT_CONNECT ;
         _sock.close() ;
         goto error ;
      }
*/
      UINT16 port = 0 ;
      rc = _ossSocket::getPort( serviceName, port ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to get port :%s", serviceName ) ;
         goto error ;
      }

      {
      _ossSocket sock( hostName, port ) ;
      rc = sock.initSocket() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to init socket:%d", rc ) ;
         goto error ;
      }
      sock.closeWhenDestruct( FALSE ) ;
      rc = sock.connect() ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "failed to connect remote[%s:%s], rc:%d",
                 hostName, serviceName, rc ) ;
         goto error ;
      }

      try
      {
         _sock.assign( tcp::v4(), sock.native() ) ; 
      }
      catch ( std::exception &e )
      {
         PD_LOG( PDERROR, "unexpected err happened:%s", e.what() ) ;
         rc = SDB_SYS ;
         sock.close() ;
         _sock.close() ;
         goto error ;
      }
      }

      setOpt() ;
   done:
      PD_TRACE_EXITRC ( SDB__NETEVNHND_SYNCCONN, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETEVNHND_ASYNCRD, "_netEventHandler::asyncRead" )
   void _netEventHandler::asyncRead()
   {
      PD_TRACE_ENTRY ( SDB__NETEVNHND_ASYNCRD ) ;
      if ( NET_EVENT_HANDLER_STATE_HEADER == _state )
      {
         async_read( _sock, buffer(&_header, sizeof(_MsgHeader)),
                     boost::bind(&_netEventHandler::_readCallback,
                                 shared_from_this(),
                                 boost::asio::placeholders::error )) ;
      }
      else
      {
         UINT32 len = _header.messageLength ;
         if ( SDB_OK != _allocateBuf( len ) )
         {
            close() ;
            _frame->handleClose( shared_from_this(), _id ) ;
            _frame->_erase( handle() ) ;
            goto done ;
         }
         ossMemcpy( _buf, &_header, sizeof( _MsgHeader ) ) ;
         async_read( _sock, buffer((CHAR *)((ossValuePtr)_buf +
                                            sizeof(_MsgHeader)),
                                    len - sizeof(_MsgHeader)),
                     boost::bind( &_netEventHandler::_readCallback,
                                  shared_from_this(),
                                  boost::asio::placeholders::error ) ) ;
      }

   done:
      PD_TRACE_EXIT ( SDB__NETEVNHND_ASYNCRD ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETEVNHND_SYNCSND, "_netEventHandler::syncSend" )
   INT32 _netEventHandler::syncSend( const void *buf,
                                     UINT32 len )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETEVNHND_SYNCSND );
      UINT32 send = 0 ;
      try
      {
         while ( send < len )
         {
            send +=  _sock.send(buffer((const void*)((ossValuePtr)buf + send),
                                        len - send));
         }
      }
      catch ( boost::system::system_error &e )
      {
         PD_LOG( PDERROR, "Failed to send to node :%d, %d, %d, %s",
                 _id.columns.groupID, _id.columns.nodeID,
                 _id.columns.serviceID, e.what() ) ;
         rc = SDB_NET_SEND_ERR ;
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETEVNHND_SYNCSND, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETEVNHND__ALLOBUF, "_netEventHandler::_allocateBuf" )
   INT32 _netEventHandler::_allocateBuf( UINT32 len )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETEVNHND__ALLOBUF );
      if ( _bufLen < len )
      {
         if ( NULL != _buf )
         {
            SDB_OSS_FREE( _buf ) ;
            _bufLen = 0 ;
         }
         _buf = (CHAR *)SDB_OSS_MALLOC( len ) ;
         if ( NULL == _buf )
         {
            PD_LOG( PDERROR, "mem allocate failed" ) ;
            rc = SDB_OOM ;
            goto error ;
         }
         _bufLen = len ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETEVNHND__ALLOBUF, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETEVNHND__RDCALLBK, "_netEventHandler::_readCallback" )
   void _netEventHandler::_readCallback( const boost::system::error_code &
                                         error )
   {
      PD_TRACE_ENTRY ( SDB__NETEVNHND__RDCALLBK ) ;
      _isInAsync = TRUE ;

      if ( error )
      {
         if ( error.value() == boost::system::errc::operation_canceled ||
              error.value() == boost::system::errc::no_such_file_or_directory )
         {
            PD_LOG ( PDINFO, "connection aborted, node:%d, %d, %d",
                     _id.columns.groupID, _id.columns.nodeID,
                     _id.columns.serviceID ) ;
         }
         else
         {
            PD_LOG ( PDERROR, "Error received, node:%d, %d, %d, err=%d",
                     _id.columns.groupID, _id.columns.nodeID,
                     _id.columns.serviceID, error.value() ) ;
         }

         goto error_close ;
      }

      if ( !_isConnected )
      {
         goto error_close ;
      }

      if ( NET_EVENT_HANDLER_STATE_HEADER == _state )
      {
         /// error header
         if ( sizeof(_MsgHeader) > (UINT32)_header.messageLength
              || NET_MSG_MAX_LEN < (UINT32)_header.messageLength )
         {
            PD_LOG( PDERROR, "Error header received, node:%d, %d, %d",
                    _id.columns.groupID, _id.columns.nodeID,
                    _id.columns.serviceID ) ;
            goto error_close ;
         }
         else
         {
            PD_LOG( PDDEBUG, "msg header: [len:%d], [opCode: [%d]%d], "
                             "[TID:%d], [groupID:%d], [nodeID:%d], "
                             "[ADDR:%s], [PORT:%d]",
                    _header.messageLength, IS_REPLY_TYPE(_header.opCode)?1:0,
                    GET_REQUEST_TYPE(_header.opCode),
                    _header.TID, _header.routeID.columns.groupID,
                    _header.routeID.columns.nodeID,
                    remoteAddr( _sock ).c_str(), remotePort( _sock ) ) ;
            /// add to route table
            if ( MSG_INVALID_ROUTEID == _id.value )
            {
               if ( MSG_INVALID_ROUTEID != _header.routeID.value )
               {
                  _id = _header.routeID ;
                  _frame->_addRoute( shared_from_this() ) ;
               }
            }
         }
         /// msg has only header
         if ( sizeof(_MsgHeader) == _header.messageLength )
         {
            if ( SDB_OK != _allocateBuf( sizeof(_MsgHeader) ))
            {
               goto error_close ;
            }
            ossMemcpy( _buf, &_header, sizeof( _MsgHeader ) ) ;
            _frame->handleMsg( shared_from_this() ) ;
            _state = NET_EVENT_HANDLER_STATE_HEADER ;
            asyncRead() ;
            goto done ;
         }

         _state = NET_EVENT_HANDLER_STATE_BODY ;
         asyncRead() ;
      }
      else
      {
         _frame->handleMsg( shared_from_this() ) ;
         _state = NET_EVENT_HANDLER_STATE_HEADER ;
         asyncRead() ;
      }

   done:
      _isInAsync = FALSE ;
      PD_TRACE_EXIT ( SDB__NETEVNHND__RDCALLBK ) ;
      return ;
   error_close:
      _isInAsync = FALSE ;
      if ( _isConnected )
      {
         close() ;
      }
      _frame->handleClose( shared_from_this(), _id ) ;
      _frame->_erase( handle() ) ;
      goto done ;
   }

}

