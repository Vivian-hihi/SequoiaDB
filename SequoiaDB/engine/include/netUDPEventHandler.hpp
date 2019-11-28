/******************************************************************************

   Copyright (C) 2011-2018 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Source File Name = netUDPEventHandler.hpp

   Descriptive Name =

   When/how to use: this program may be used on binary and text-motionatted
   versions of PD component. This file contains declare of PD functions.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/01/2019  HGM Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef NET_UDP_EVENT_HANDLER_HPP_
#define NET_UDP_EVENT_HANDLER_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "netDef.hpp"
#include "ossLatch.hpp"
#include "ossAtomic.hpp"
#include "utilPooledObject.hpp"
#include "netEventHandlerBase.hpp"
#include "ossMemPool.hpp"

namespace engine
{

   // remote testing status timeout at each 2 minutes
   #define NET_UDP_REMOTE_TIMEOUT      ( 120 * OSS_ONE_SEC )
   // after 5 times without receiving message, mark the UDP unavailable
   #define NET_UDP_REMOTE_UNAVAILABLE  ( 5 )
   /*
      _netUDPEventHandler define
    */
   class _netUDPEventHandler : public netEventHandlerBase
   {
   public:
      _netUDPEventHandler( NET_UDP_EV_SUIT evSuit,
                           const NET_HANDLE &handle,
                           const MsgRouteID &routeID,
                           const netUDPEndPoint &endPoint ) ;
      virtual ~_netUDPEventHandler() ;

      static NET_EH createShared( NET_UDP_EV_SUIT evSuit,
                                  const NET_HANDLE &handle,
                                  const MsgRouteID &routeID,
                                  const netUDPEndPoint &endPoint ) ;

   public:
      OSS_INLINE virtual NET_EVENT_HANDLER_TYPE getHandlerType() const
      {
         return NET_EVENT_HANDLER_UDP ;
      }

      virtual INT32 syncConnect( const CHAR *hostName,
                                 const CHAR *serviceName ) ;
      virtual void asyncRead() ;
      virtual INT32 syncSend( const void *buf, UINT32 len ) ;
      virtual void close() ;
      virtual void setOpt() ;
      virtual CHAR *msg() ;

      virtual std::string localAddr() const ;
      virtual std::string remoteAddr() const ;
      virtual UINT16 localPort() const ;
      virtual UINT16 remotePort() const ;

      void readCallback( const MsgHeader *message ) ;
      void setRouteID( const MsgRouteID &routeID ) ;

      OSS_INLINE BOOLEAN isRemoteValidated()
      {
         return !_testRemote ;
      }

      OSS_INLINE BOOLEAN isRemoteUnavailable()
      {
         return _testRemoteCount > NET_UDP_REMOTE_UNAVAILABLE ;
      }

      OSS_INLINE void increaseRemoteTest()
      {
         _testRemote = ( ( ++ _testRemoteCount ) >
                         NET_UDP_REMOTE_UNAVAILABLE ) ? TRUE : FALSE ;
      }

      OSS_INLINE void setRemoteValidated()
      {
         _testRemote = FALSE ;
         _testRemoteCount = 0 ;
      }

      BOOLEAN isBeatTimeout( UINT32 beatInterval ) ;

   protected:
      OSS_INLINE NET_UDP_EH _getShared()
      {
         return NET_UDP_EH::makeRaw( this, ALLOC_POOL ) ;
      }

   protected:
      NET_UDP_EV_SUIT   _mainSuit ;
      netUDPEndPoint    _remoteEndPoint ;

      // need test remote status, which might not support UDP
      // 1. at the begining, send beat to test
      // 2. if received UDP message, mark remote validated
      // 3. if not received UDP message after 10 test beats, mark
      //    remove unavailable
      // 4. will re-test after 120 seconds
      BOOLEAN           _testRemote ;
      INT32             _testRemoteCount ;
   } ;

}

#endif // NET_UDP_EVENT_HANDLER_HPP_
