/******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netEventHandler.hpp

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

#ifndef NETEVENTHANDLER_HPP_
#define NETEVENTHANDLER_HPP_

#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "core.hpp"
#include "oss.hpp"
#include "netDef.hpp"
#include "ossLatch.hpp"
#include "ossEvent.hpp"

using namespace boost::asio ;

namespace engine
{
   enum NET_EVENT_HANDLER_STATE
   {
      NET_EVENT_HANDLER_STATE_HEADER = 0,
      NET_EVENT_HANDLER_STATE_BODY = 1,
   } ;

   class _netFrame ;

   class _netEventHandler :
         public boost::enable_shared_from_this<_netEventHandler>,
         public SDBObject
   {
      public:
         _netEventHandler( _netFrame *frame ) ;

         ~_netEventHandler() ;

      public:
         inline void id( const _MsgRouteID &id )
         {
            _id = id ;
         }
         inline const _MsgRouteID &id()
         {
            return _id ;
         }
         inline boost::asio::ip::tcp::socket &socket()
         {
            return _sock ;
         }
         inline _ossSpinXLatch &mtx()
         {
            return _mtx ;
         }
         inline NET_HANDLE handle()
         {
            return _handle ;
         }
         CHAR *msg()
         {
            return _buf ;
         }
         inline NET_EVENT_HANDLER_STATE state()
         {
            return _state ;
         }
         inline void close()
         {
            _mtx.get() ;
            _isConnected = FALSE ;
            while ( _isInAsync )
            {
               ossSleep( 50 ) ;
            }
            _sock.close() ;
            _mtx.release() ;
         }

      public:
         void asyncRead() ;

         INT32 syncConnect( const CHAR *hostName,
                            const CHAR *serviceName ) ;

         INT32 syncSend( const void *buf,
                         UINT32 len ) ;

         void setOpt() ;

      private:
         void _readCallback(const boost::system::error_code &
                            error ) ;

         INT32 _allocateBuf( UINT32 len ) ;

      private:
         boost::asio::ip::tcp::socket     _sock ;
         _ossSpinXLatch                   _mtx ;
         _MsgHeader                       _header ;
         CHAR                             *_buf ;
         UINT32                           _bufLen ;
         NET_EVENT_HANDLER_STATE          _state ;
         _MsgRouteID                      _id ;
         _netFrame                        *_frame ;
         NET_HANDLE                       _handle ;
         BOOLEAN                          _isConnected ;
         BOOLEAN                          _isInAsync ;

   };

   typedef boost::shared_ptr<_netEventHandler> NET_EH ;
}

#endif

