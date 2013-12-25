/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netFrame.cpp

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

#include "core.hpp"
#include "netFrame.hpp"
#include "netMsgHandler.hpp"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "netTrace.hpp"
#include <boost/bind.hpp>

using namespace boost::asio::ip ;

namespace engine
{
   #define NET_INSERT_OPPO( a )\
           _opposite.insert(make_pair( a->handle(), a))
   #define NET_INSERT_ROUTE( a )\
           _route.insert(make_pair(a->id().value, a))
   #define NET_LISTEN_HOST "0.0.0.0"

   typedef multimap<UINT64, NET_EH>::iterator MULTI_ITR ;

   _netFrame::_netFrame( _netMsgHandler *handler ):
                         _handler(handler),
                         _acceptor(_ioservice),
                         _handle(1),
                         _timerID(0)
   {
      _local.value = MSG_INVALID_ROUTEID ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_DECONS, "_netFrame::~_netFrame" )
   _netFrame::~_netFrame()
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME_DECONS );
      stop() ;
      _route.clear() ;
      _timers.clear() ;
      _opposite.clear() ;
      PD_TRACE_EXIT ( SDB__NETFRAME_DECONS );
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_RUN, "_netFrame::run" )
   void _netFrame::run()
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME_RUN );
      if ( _acceptor.is_open() )
      {
         _asyncAccept() ;
      }

      _ioservice.run() ;
      PD_TRACE_EXIT( SDB__NETFRAME_RUN );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_STOP, "_netFrame::stop" )
   void _netFrame::stop()
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME_STOP );
      close() ;
      _ioservice.stop() ;
      _acceptor.close() ;
      PD_TRACE_EXIT ( SDB__NETFRAME_STOP );
   }

   UINT32 _netFrame::getLocalAddress()
   {
      UINT32 ip = 0 ;
      boost::asio::io_service io_srv ;
      tcp::resolver resolver( io_srv ) ;
      tcp::resolver::query query( boost::asio::ip::host_name(), "") ;
      tcp::resolver::iterator itr = resolver.resolve( query ) ;
      tcp::resolver::iterator end ;
      for ( ; itr != end; itr++ )
      {
         tcp::endpoint ep = *itr ;
         if ( ep.address().is_v4() )
         {
            ip = ep.address().to_v4().to_ulong() ;
            break ;
         }
      }

      return ip ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_LISTEN, "_netFrame::listen" )
   INT32 _netFrame::listen( const CHAR *hostName,
                            const CHAR *serviceName )
   {
      SDB_ASSERT( NULL != hostName, "hostName should not be NULL" )
      SDB_ASSERT( NULL != serviceName, "serviceName should not be NULL" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_LISTEN );
      if ( _acceptor.is_open() )
      {
         rc = SDB_NET_ALREADY_LISTENED ;
         goto error ;
      }

      try
      {
         /// here we bind 0.0.0.0.
         tcp::resolver::query query ( tcp::v4(), NET_LISTEN_HOST, serviceName ) ;
         tcp::resolver resolver ( _ioservice ) ;
         tcp::resolver::iterator itr = resolver.resolve ( query ) ;
         ip::tcp::endpoint endpoint = *itr ;
         _acceptor.open( endpoint.protocol() ) ;
         _acceptor.set_option(tcp::acceptor::reuse_address(TRUE)) ;
         _acceptor.bind( endpoint ) ;
         _acceptor.listen() ;
      }
      catch ( boost::system::system_error &e )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
              "Failed to listen  %s: %s: %s", hostName, serviceName,
              e.what() ) ;
         rc = SDB_NET_CANNOT_LISTEN ;
         goto error ;
      }
      PD_LOG( PDDEBUG, "listening on port %s", serviceName ) ;

   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_LISTEN, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_SYNNCCONN, "_netFrame::syncConnect" )
   INT32 _netFrame::syncConnect( const CHAR *hostName,
                                 const CHAR *serviceName,
                                 const _MsgRouteID &id )
   {
      SDB_ASSERT( NULL != hostName, "hostName should not be NULL" )
      SDB_ASSERT( NULL != serviceName, "serviceName should not be NULL" )

      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_SYNNCCONN );
      _netEventHandler *ev = SDB_OSS_NEW _netEventHandler( this ) ;
      if ( NULL == ev )
      {
         PD_LOG ( PDERROR, "Failed to malloc mem" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      {
      NET_EH eh( ev ) ;
      rc = eh->syncConnect( hostName, serviceName ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      eh->id( id ) ;
      _mtx.get() ;
      NET_INSERT_OPPO( eh ) ;
      NET_INSERT_ROUTE( eh ) ;
      _mtx.release() ;
      eh->asyncRead() ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_SYNNCCONN, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_SYNCSEND, "_netFrame::syncSend" )
   INT32 _netFrame::syncSend( const _MsgRouteID &id,
                              void *header )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL")
      SDB_ASSERT( MSG_INVALID_ROUTEID != id.value,
                  "id.value should not be zero" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_SYNCSEND );
      NET_EH eh;
      _mtx.get_shared() ;
      MULTI_ITR itr =  _route.find( id.value ) ;
      if ( _route.end() == itr )
      {
         _mtx.release_shared() ;
         rc = SDB_NET_NOT_CONNECT ;
         goto error ;
      }
      eh = itr->second ;
      _mtx.release_shared() ;
      {
      _MsgHeader *msgHeader = ( _MsgHeader * )header ;
      if ( MSG_INVALID_ROUTEID == msgHeader->routeID.value )
      {
         msgHeader->routeID = _local ;
      }
      eh->mtx().get() ;
      rc = eh->syncSend( msgHeader, msgHeader->messageLength ) ;
      eh->mtx().release() ;
      if ( SDB_OK != rc )
      {
         eh->close() ;
         goto error ;
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_SYNCSEND, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_SYNCSEND2, "_netFrame::syncSend" )
   INT32 _netFrame::syncSend( const NET_HANDLE &handle,
                              void *header )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL")
      SDB_ASSERT( NET_INVALID_HANDLE != handle,
                  "handle should not be invalid" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_SYNCSEND2 );
      NET_EH eh ;
      _mtx.get_shared() ;
      map<NET_HANDLE, NET_EH>::iterator itr =
                                _opposite.find( handle ) ;
      if ( _opposite.end() == itr )
      {
         _mtx.release_shared() ;
         rc = SDB_NET_INVALID_HANDLE ;
         goto error ;
      }
      eh = itr->second ;
      _mtx.release_shared() ;
      {
      _MsgHeader *msgHeader = ( _MsgHeader * )header ;
      if ( MSG_INVALID_ROUTEID == msgHeader->routeID.value )
      {
         msgHeader->routeID = _local ;
      }
      eh->mtx().get() ;
      rc = eh->syncSend( msgHeader, msgHeader->messageLength ) ;
      eh->mtx().release() ;
      if ( SDB_OK != rc )
      {
         eh->close() ;
         goto error ;
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_SYNCSEND2, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_SYNCSEND3, "INT32 _netFrame::syncSend" )
   INT32 _netFrame::syncSend( const NET_HANDLE &handle,
                              MsgHeader *header,
                              const void *body,
                              UINT32 bodyLen )
   {
      SDB_ASSERT( NULL != header, "header should not be NULL")
      SDB_ASSERT( NULL != body, "body should not be NULL")
      SDB_ASSERT( NET_INVALID_HANDLE != handle,
                  "handle should not be invalid" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_SYNCSEND3 );
      NET_EH eh ;
      _mtx.get_shared() ;
      map<NET_HANDLE, NET_EH>::iterator itr =
                                _opposite.find( handle ) ;
      if ( _opposite.end() == itr )
      {
         _mtx.release_shared() ;
         rc = SDB_NET_INVALID_HANDLE ;
         goto error ;
      }
      eh = itr->second ;
      _mtx.release_shared() ;

      if ( MSG_INVALID_ROUTEID == header->routeID.value )
      {
         header->routeID = _local ;
      }
      eh->mtx().get() ;
      /// header len should be computed. can not get sizeof(MsgHeader)
      rc = eh->syncSend( header, header->messageLength - bodyLen ) ;
      if ( SDB_OK != rc )
      {
         eh->mtx().release() ;
         eh->close() ;
         goto error ;
      }
      rc = eh->syncSend( body,
                         bodyLen ) ;
      eh->mtx().release() ;
      if ( SDB_OK != rc )
      {
         eh->close() ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_SYNCSEND3, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_SYNCSEND4, "_netFrame::syncSend" )
   INT32 _netFrame::syncSend( const  _MsgRouteID &id,
                              MsgHeader *header,
                              const void *body,
                              UINT32 bodyLen )
   {
      SDB_ASSERT( NULL != header && NULL != body, "should not be NULL")
      SDB_ASSERT( MSG_INVALID_ROUTEID != id.value,
                  "id.value should not be zero" )
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_SYNCSEND4 );
      NET_EH eh;
      _mtx.get_shared() ;
      MULTI_ITR itr =  _route.find( id.value ) ;
      if ( _route.end() == itr )
      {
         _mtx.release_shared() ;
         rc = SDB_NET_NOT_CONNECT ;
         goto error ;
      }
      eh = itr->second ;
      _mtx.release_shared() ;
      if ( MSG_INVALID_ROUTEID == header->routeID.value )
      {
         header->routeID = _local ;
      }
      eh->mtx().get() ;
      rc = eh->syncSend( header, header->messageLength - bodyLen ) ;
      if ( SDB_OK != rc )
      {
         eh->mtx().release() ;
         eh->close() ;
         goto error ;
      }
      rc = eh->syncSend( body,
                         bodyLen ) ;
      eh->mtx().release() ;
      if ( SDB_OK != rc )
      {
         eh->close() ;
         goto error ;
      }
   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_SYNCSEND4, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_SYNCSENDV, "_netFrame::syncSendv" )
   INT32 _netFrame::syncSendv( const _MsgRouteID &id,
                               MsgHeader *header,
                               const netIOVec &iov )
   {
      SDB_ASSERT( NULL != header, "should not be NULL" )
      SDB_ASSERT( MSG_INVALID_ROUTEID != id.value,
                  "id.value should not be zero" )
      PD_TRACE_ENTRY( SDB__NETFRAME_SYNCSENDV ) ;
      INT32 rc = SDB_OK ;
#ifdef _DEBUG
      UINT32 totalLen = 0 ;
      for ( netIOVec::const_iterator itr = iov.begin();
            itr != iov.end();
            itr++ )
      {
         SDB_ASSERT( NULL != itr->iovBase, "should not be NULL" )
         totalLen += itr->iovLen ;
      }

      if ( totalLen + sizeof(MsgHeader) != header->messageLength )
      {
         PD_LOG( PDERROR, "the length in header[%d] not equal to"
                 " the whole msg's len[%d]",
                  header->messageLength,
                  totalLen + sizeof(MsgHeader)) ;
         rc = SDB_INVALIDARG ;
         goto error ;
      }
#endif
      {
      NET_EH eh;
      _mtx.get_shared() ;
      MULTI_ITR itr =  _route.find( id.value ) ;
      if ( _route.end() == itr )
      {
         _mtx.release_shared() ;
         rc = SDB_NET_NOT_CONNECT ;
         goto error ;
      }
      eh = itr->second ;
      _mtx.release_shared() ;
      if ( MSG_INVALID_ROUTEID == header->routeID.value )
      {
         header->routeID = _local ;
      }

      eh->mtx().get() ;
      rc = eh->syncSend( header, sizeof(MsgHeader) ) ;
      if ( SDB_OK != rc )
      {
         eh->mtx().release() ;
         eh->close() ;
         goto error ;
      }

      for ( netIOVec::const_iterator itr = iov.begin();
            itr != iov.end();
            itr++ )
      {
         rc = eh->syncSend( itr->iovBase, itr->iovLen ) ;
         if ( SDB_OK != rc )
         {
            eh->mtx().release() ;
            eh->close() ;
            goto error ;
         }
      }

      eh->mtx().release() ;
      }
   done:
      PD_TRACE_EXITRC( SDB__NETFRAME_SYNCSENDV, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_CLOSE, "_netFrame::close" )
   void _netFrame::close( const _MsgRouteID &id )
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME_CLOSE );
      _mtx.get_shared() ;
      pair<MULTI_ITR, MULTI_ITR> pitr = _route.equal_range( id.value ) ;
      for ( MULTI_ITR mitr=pitr.first;
            mitr != pitr.second;
            mitr++ )
      {
         mitr->second->close() ;
      }
      _mtx.release_shared() ;
      PD_TRACE_EXIT ( SDB__NETFRAME_CLOSE );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_CLOSE2, "_netFrame::close" )
   void _netFrame::close()
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME_CLOSE2 );
      _mtx.get_shared() ;
      map<NET_HANDLE, NET_EH>::iterator itr =
                                 _opposite.begin() ;
      for ( ; itr != _opposite.end(); itr++ )
      {
         itr->second->close() ;
      }
      _mtx.release_shared() ;

      PD_TRACE_EXIT ( SDB__NETFRAME_CLOSE2 );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION( SDB__NETFRAME_CLOSE3, "_netFrame::close" )
   void _netFrame::close( const NET_HANDLE &handle )
   {
      PD_TRACE_ENTRY( SDB__NETFRAME_CLOSE3 ) ;

      _mtx.get_shared() ;
      map<NET_HANDLE, NET_EH>::iterator itr =
                                  _opposite.find( handle ) ;
      if ( _opposite.end() != itr )
      {
         itr->second->close() ;
         _mtx.release_shared() ;
      }
      else
      {
         _mtx.release_shared() ;
         PD_LOG( PDERROR, "invalid net handle:%d", handle ) ;
      }

      PD_TRACE_EXIT( SDB__NETFRAME_CLOSE3 ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_ADDTIMER, "_netFrame::addTimer" )
   INT32 _netFrame::addTimer( UINT32 millsec, _netTimeoutHandler *handler,
                              UINT32 &timerid )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_ADDTIMER );
      _mtx.get() ;

      _netTimer *t = SDB_OSS_NEW _netTimer( millsec,
                                            ++_timerID,
                                            _ioservice,
                                            handler ) ;
      if ( NULL == t )
      {
         rc = SDB_OOM ;
         goto error ;
      }
      {
         NET_TH timer(t) ;
         _timers.insert( std::make_pair(timer->id(), timer ));
         _mtx.release() ;
         timerid = timer->id() ;
         timer->asyncWait() ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETFRAME_ADDTIMER, rc );
      return rc ;
   error:
      _mtx.release() ;
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_REMTIMER, "_netFrame::removeTimer" )
   INT32 _netFrame::removeTimer( UINT32 id )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETFRAME_REMTIMER );
      _mtx.get() ;
      map<UINT32, NET_TH>::iterator itr=
                                 _timers.find( id ) ;
      if ( _timers.end() == itr )
      {
         rc = SDB_NET_TIMER_ID_NOT_FOUND ;
         goto error ;
      }

      itr->second->cancel() ;
      _timers.erase(itr) ;
   done:
      _mtx.release() ;
      PD_TRACE_EXITRC ( SDB__NETFRAME_REMTIMER, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME_HNDMSG, "_netFrame::handleMsg" )
   void _netFrame::handleMsg( NET_EH eh )
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME_HNDMSG );
      INT32 rc = _handler->handleMsg( eh->handle(),
                                      (_MsgHeader *)eh->msg(),
                                      eh->msg() ) ;
      if ( SDB_NET_BROKEN_MSG == rc )
      {
         eh->close() ;
      }
      PD_TRACE1 ( SDB__NETFRAME_HNDMSG, PD_PACK_INT(rc) );
      PD_TRACE_EXIT ( SDB__NETFRAME_HNDMSG );
      return ;
   }

   void _netFrame::handleClose( NET_EH eh , _MsgRouteID id)
   {
      _handler->handleClose( eh->handle(), id ) ;
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME__ADDRT, "_netFrame::_addRoute" )
   void _netFrame::_addRoute( NET_EH eh )
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME__ADDRT );
      _mtx.get() ;
      NET_INSERT_ROUTE( eh ) ;
      _mtx.release() ;
      PD_TRACE_EXIT ( SDB__NETFRAME__ADDRT );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME__ASYNCAPT, "_netFrame::_asyncAccept" )
   void _netFrame::_asyncAccept()
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME__ASYNCAPT );
      NET_EH handler( SDB_OSS_NEW _netEventHandler(this) ) ;
      _acceptor.async_accept(handler->socket(),
                             boost::bind(&_netFrame::_acceptCallback,
                                         this,
                                         handler,
                                         boost::asio::placeholders::error)) ;
      PD_TRACE_EXIT ( SDB__NETFRAME__ASYNCAPT );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME__APTCALLBCK, "_netFrame::_acceptCallback" )
   void _netFrame::_acceptCallback( NET_EH eh,
                                    const boost::system::error_code &
                                    error )
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME__APTCALLBCK );
      if ( error )
      {
         PD_LOG ( PDERROR, "Error received when handling accept" ) ;
         return ;
      }

      eh->setOpt() ;
      _mtx.get() ;
      NET_INSERT_OPPO(eh) ;
      _mtx.release() ;
      _asyncAccept() ;
      eh->asyncRead() ;
      PD_TRACE_EXIT ( SDB__NETFRAME__APTCALLBCK );
      return ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETFRAME__ERASE, "_netFrame::_erase" )
   void _netFrame::_erase( const NET_HANDLE &handle )
   {
      PD_TRACE_ENTRY ( SDB__NETFRAME__ERASE );
      _mtx.get() ;
      map<NET_HANDLE, NET_EH>::iterator itr =
                                _opposite.find( handle ) ;
      if ( _opposite.end() == itr )
      {
         goto done ;
      }
      {
      pair<MULTI_ITR, MULTI_ITR> pitr = _route.equal_range(
                                        itr->second->id().value) ;
      for ( MULTI_ITR mitr=pitr.first;
            mitr != pitr.second;
            mitr++ )
      {
         if ( mitr->second->handle() ==
              handle )
         {
            _route.erase( mitr ) ;
            break ;
         }
      }
      _opposite.erase( itr ) ;
      }
   done:
      _mtx.release() ;
      PD_TRACE_EXIT ( SDB__NETFRAME__ERASE );
      return ;
   }

}
