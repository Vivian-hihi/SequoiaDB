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

   Source File Name = netUDPEventHandler.cpp

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

#include "core.hpp"
#include "netUDPEventHandler.hpp"
#include "netUDPEventSuit.hpp"
#include "netFrame.hpp"
#include "netRoute.hpp"
#include "ossMem.hpp"
#include "pmdEnv.hpp"
#include "msgDef.h"
#include "pd.hpp"
#include "pdTrace.hpp"
#include "netTrace.hpp"
#include "msgMessageFormat.hpp"
#include <boost/bind.hpp>

using namespace boost::asio::ip ;
using namespace std ;

namespace engine
{

   /*
      _netUDPEventHandler implement
    */
   _netUDPEventHandler::_netUDPEventHandler( NET_UDP_EV_SUIT evSuit,
                                             const NET_HANDLE &handle,
                                             const MsgRouteID &routeID,
                                             const netUDPEndPoint &endPoint )
   : _netEventHandlerBase( evSuit->getFrame()->getMainSuit(), handle ),
     _mainSuit( evSuit ),
     _remoteEndPoint( endPoint ),
     _testRemote( TRUE ),
     _testRemoteCount( 0 )
   {
      setRouteID( routeID ) ;

      _isConnected = TRUE ;
      _isNew = FALSE ;
   }

   _netUDPEventHandler::~_netUDPEventHandler()
   {
   }

   NET_EH _netUDPEventHandler::createShared( NET_UDP_EV_SUIT evSuit,
                                             const NET_HANDLE &handle,
                                             const MsgRouteID &routeID,
                                             const netUDPEndPoint &endPoint )
   {
      NET_EH eh ;

      NET_UDP_EH tmpEH = NET_UDP_EH::allocRaw( ALLOC_POOL ) ;
      if ( NULL != tmpEH.get() &&
           NULL != new( tmpEH.get() ) netUDPEventHandler( evSuit,
                                                          handle,
                                                          routeID,
                                                          endPoint ) )
      {
         eh = NET_EH::makeRaw( tmpEH.get(), ALLOC_POOL ) ;
      }

      return eh ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETUDPEVNHND_SYNCCONNECT, "_netUDPEventHandler::syncConnect" )
   INT32 _netUDPEventHandler::syncConnect( const CHAR *hostName,
                                           const CHAR *serviceName )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__NETUDPEVNHND_SYNCCONNECT ) ;

      PD_CHECK( NULL != _mainSuit.get() && _mainSuit->isOpened(),
                SDB_NETWORK, error, PDERROR, "Failed to send UDP message to "
                "%s:%s, UDP suit is not opened" ) ;

   done:
      PD_TRACE_EXITRC( SDB__NETUDPEVNHND_SYNCCONNECT, rc ) ;
      return rc ;

   error:
      goto done ;
   }

   void _netUDPEventHandler::asyncRead()
   {
      // do nothing
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETUDPEVNHND_SYNCSEND, "_netUDPEventHandler::syncSend" )
   INT32 _netUDPEventHandler::syncSend( const void *buf, UINT32 len )
   {
      INT32 rc = SDB_OK ;

      PD_TRACE_ENTRY( SDB__NETUDPEVNHND_SYNCSEND ) ;

      PD_CHECK( _isConnected, SDB_NETWORK, error, PDWARNING,
                "Failed to send message via UDP handle [%u], it is not valid",
                _handle ) ;

      /// not care send suc or failed
      _lastSendTick = pmdGetDBTick() ;
      ++_totalIOTimes ;

      rc = _mainSuit->syncSend( _remoteEndPoint, buf, len ) ;
      PD_RC_CHECK( rc, PDERROR, "Failed to send message to %s via UDP "
                   "handle [%u], rc: %d", routeID2String( _id ).c_str(),
                   _handle, rc ) ;

      increaseRemoteTest() ;

   done:
      PD_TRACE_EXITRC( SDB__NETUDPEVNHND_SYNCSEND, rc ) ;
      return rc ;

   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETUDPEVNHND_CLOSE, "_netUDPEventHandler::close" )
   void _netUDPEventHandler::close()
   {
      PD_TRACE_ENTRY( SDB__NETUDPEVNHND_CLOSE ) ;

      _isConnected = FALSE ;

      PD_TRACE_EXIT( SDB__NETUDPEVNHND_CLOSE ) ;
   }

   void _netUDPEventHandler::setOpt()
   {
      // do nothing
   }

   CHAR *_netUDPEventHandler::msg()
   {
      return _mainSuit->getMessage() ;
   }

   string _netUDPEventHandler::localAddr() const
   {
      boost::system::error_code error ;
      string addr ;
      netUDPEndPoint localEndPoint = _mainSuit->getLocalEndPoint() ;
      addr = localEndPoint.address().to_string( error ) ;
      if ( error )
      {
         PD_LOG( PDERROR, "Failed to get local address, occurred error: %s",
                 error.message().c_str() ) ;
      }
      return addr ;
   }

   string _netUDPEventHandler::remoteAddr() const
   {
      boost::system::error_code error ;
      string addr ;
      addr = _remoteEndPoint.address().to_string( error ) ;
      if ( error )
      {
         PD_LOG( PDERROR, "Failed to get local address, occurred error: %s",
                 error.message().c_str() ) ;
      }
      return addr ;
   }

   UINT16 _netUDPEventHandler::localPort() const
   {
      return _mainSuit->getLocalEndPoint().port() ;
   }

   UINT16 _netUDPEventHandler::remotePort() const
   {
      return _remoteEndPoint.port() ;
   }

   void _netUDPEventHandler::readCallback( const MsgHeader *message )
   {
      _lastRecvTick = pmdGetDBTick() ;
      _lastBeatTick = _lastRecvTick ;

      PD_LOG( PDDEBUG, "UDP connection[Handle:%d] received "
              "message[%s] from %s:%d", _handle,
              msg2String( message, MSG_MASK_ALL, 0 ).c_str(),
              _remoteEndPoint.address().to_string().c_str(),
              _remoteEndPoint.port() ) ;

      if ( !_isConnected )
      {
         _isConnected = TRUE ;
      }

      // received from remote, validate remote
      setRemoteValidated() ;

      _mainSuit->handleMsg( _getSharedBase() ) ;
   }

   void _netUDPEventHandler::setRouteID( const MsgRouteID &routeID )
   {
      if ( MSG_INVALID_ROUTEID == _id.value &&
           MSG_INVALID_ROUTEID != routeID.value )
      {
         id( routeID ) ;
      }
   }

   BOOLEAN _netUDPEventHandler::isBeatTimeout( UINT32 beatInterval )
   {
      if ( isRemoteValidated() )
      {
         // if remote is mark UDP validated, re-test after 120 seconds without
         // message received
         return pmdGetTickSpanTime( _lastRecvTick ) >= NET_UDP_REMOTE_TIMEOUT ;
      }
      else if ( isRemoteUnavailable() )
      {
         // if remote is mark UDP unavailable, re-test after 120 seconds
         // NOTE: 120 seconds after last test beat send
         return pmdGetTickSpanTime( _lastSendTick ) > NET_UDP_REMOTE_TIMEOUT ;
      }
      // under testing, use the default beat interval
      return pmdGetTickSpanTime( _lastBeatTick ) >= beatInterval ;
   }

}
