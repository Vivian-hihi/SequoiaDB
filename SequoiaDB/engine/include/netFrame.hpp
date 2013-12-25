/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netFrame.hpp

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

#ifndef NETFRAME_HPP_
#define NETFRAME_HPP_

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

#include "core.hpp"
#include "oss.hpp"
#include "netDef.hpp"
#include "ossLatch.hpp"
#include "netEventHandler.hpp"
#include "netTimer.hpp"
#include "ossAtomic.hpp"

using namespace std ;
namespace engine
{
   class _netMsgHandler ;

   class _netFrame : public SDBObject
   {
      public:
         /// handler will not be freed by frame
         _netFrame( _netMsgHandler *handler ) ;

         ~_netFrame() ;

      public:
         inline io_service &ioservice()
         {
            return _ioservice ;
         }

         inline NET_HANDLE allocateHandle()
         {
            return _handle.inc() ;
         }

         inline void setLocal( const MsgRouteID &id )
         {
            _local = id ;
         }

      public:
         static UINT32 getLocalAddress() ;

      public:
         void run();

         void stop() ;

         /// can only be called for once. non-reentrant
         INT32 listen( const CHAR *hostName,
                       const CHAR *serviceName ) ;

//         INT32 asyncConnect( const CHAR *hostName,
//                             const CHAR *serviceName,
//                             const NET_HANDLE &id ) ;

         /// if call this func with same params for twice,
         /// will create two connections.
         /// the connection will be maintained until the
         /// disconnect happens.
         INT32 syncConnect( const CHAR *hostName,
                            const CHAR *serviceName,
                            const _MsgRouteID &id ) ;

         INT32 syncSend( const NET_HANDLE &handle,
                          void *header ) ;

         INT32 syncSend( const  _MsgRouteID &id,
                         void *header ) ;

         INT32 syncSend( const NET_HANDLE &handle,
                         MsgHeader *header,
                         const void *body,
                         UINT32 bodyLen ) ;

         INT32 syncSend( const _MsgRouteID &id,
                         MsgHeader *header,
                         const void *body,
                         UINT32 bodyLen ) ;

         INT32 syncSendv( const _MsgRouteID &id,
                          MsgHeader *header,
                          const netIOVec &iov ) ;

         /// frame will not release handler for ever
         INT32 addTimer( UINT32 millsec, _netTimeoutHandler *handler,
                         UINT32 &timerid );

         INT32 removeTimer( UINT32 timerid ) ;

         void close( const _MsgRouteID &id ) ;

         void close( const NET_HANDLE &handle ) ;

         void close() ;

         void handleMsg( NET_EH eh ) ;

         void handleClose( NET_EH eh, _MsgRouteID id ) ;

         friend  class _netEventHandler ;

      private:
         void _asyncAccept() ;
         void _acceptCallback( NET_EH eh,
                               const boost::system::error_code &
                               error ) ;

         void _erase( const NET_HANDLE &handle ) ;

         void _addRoute( NET_EH eh ) ;
      private:
         io_service                       _ioservice ;
         multimap<UINT64, NET_EH>         _route ;
         map<NET_HANDLE, NET_EH>          _opposite ;
         map<UINT32, NET_TH>              _timers ;
         _netMsgHandler                   *_handler ;
         MsgRouteID                       _local ;
         _ossSpinSLatch                   _mtx ;
         boost::asio::ip::tcp::acceptor   _acceptor ;
         _ossAtomic32                     _handle ;
         UINT32                           _timerID;

   } ;

}

#endif

