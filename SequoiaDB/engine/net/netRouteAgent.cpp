/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

   Source File Name = netRouteAgent.cpp

   Descriptive Name = Problem Determination Header

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

#include "netRouteAgent.hpp"
#include "pdTrace.hpp"
#include "netTrace.hpp"

namespace engine
{
   _netRouteAgent::_netRouteAgent( _netMsgHandler *handler ):
                                   _frame( handler )
   {

   }

   // this updateRoute only change the old routeID to new one. It does NOT
   // change the hostname and servicename, so we do not need to restart services
   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_UPRT, "_netRouteAgent::updateRoute" )
   INT32 _netRouteAgent::updateRoute ( const _MsgRouteID &oldID,
                                       const _MsgRouteID &newID )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_UPRT );
      rc = _route.update ( oldID, newID ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done :
      PD_TRACE_EXITRC ( SDB__NETRTAG_UPRT, rc );
      return rc ;
   error :
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_UPRT2, "_netRouteAgent::updateRoute" )
   INT32 _netRouteAgent::updateRoute( const _MsgRouteID &id,
                                     const CHAR *host,
                                     const CHAR *service )
   {
      INT32 rc = SDB_OK ;
      BOOLEAN newAdd = FALSE ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_UPRT2 );
      rc = _route.update( id, host, service, &newAdd ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      // new node don't close the exist connect
      if ( FALSE == newAdd )
      {
         _frame.close( id ) ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETRTAG_UPRT2, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_UPRT3, "_netRouteAgent::updateRoute" )
   INT32 _netRouteAgent::updateRoute( const _MsgRouteID &id,
                                      const _netRouteNode &node )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_UPRT3 );
      rc = _route.update( id, node ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
      _frame.close( id ) ;

   done:
      PD_TRACE_EXITRC ( SDB__NETRTAG_UPRT3, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_LSTN, "_netRouteAgent::listen" )
   INT32 _netRouteAgent::listen( const _MsgRouteID &id )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_LSTN );
      _netRouteNode node ;
      CHAR host[ OSS_MAX_HOSTNAME + 1 ] ;
      CHAR service[ OSS_MAX_SERVICENAME + 1] ;
      rc = _route.route( id, host, service ) ;
      if ( SDB_OK != rc )
      {
         PD_LOG( PDERROR, "can not find the route of %d, %d, %d",
                 id.columns.groupID, id.columns.nodeID,
                 id.columns.serviceID ) ;
         goto error ;
      }

      rc = _frame.listen( host, service ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETRTAG_LSTN, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_SYNCSND, "_netRouteAgent::syncSend" )
   INT32 _netRouteAgent::syncSend( const _MsgRouteID &id,
                                   void *header )
   {
      SDB_ASSERT( NULL != header,
                  "should not be NULL" ) ;

      /// todo: trans to _netFrame
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_SYNCSND );
      rc = _frame.syncSend( id, header ) ;
      if ( SDB_OK == rc )
      {
         goto done ;
      }
      else if ( SDB_NET_NOT_CONNECT != rc )
      {
         goto error ;
      }
      else
      {
         /// do nothing
      }
      {
      CHAR host[ OSS_MAX_HOSTNAME + 1 ] ;
      CHAR service[ OSS_MAX_SERVICENAME + 1] ;
      rc = _route.route( id, host, service ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _frame.syncConnect( host,
                               service,
                               id ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }

      rc = _frame.syncSend( id, header ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
      }
   done:
      PD_TRACE_EXITRC ( SDB__NETRTAG_SYNCSND, rc );
      return rc ;
   error:
      goto done ;
   }

   INT32 _netRouteAgent::syncSend( const NET_HANDLE &handle,
                                   void *header )
   {
     SDB_ASSERT( NULL != header,
                  "should not be NULL" ) ;

      return _frame.syncSend( handle, header ) ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_SYNCSND2, "_netRouteAgent::syncSend" )
   INT32 _netRouteAgent::syncSend( const _MsgRouteID &id,
                                   MsgHeader *header, void *body,
                                   UINT32 bodyLen )
   {
      SDB_ASSERT( NULL != header && NULL != body,
                  "should not be NULL" ) ;
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_SYNCSND2 );
      rc = _frame.syncSend( id, header, body, bodyLen ) ;
      if ( SDB_OK == rc )
      {
         goto done ;
      }
      else if ( SDB_NET_NOT_CONNECT != rc )
      {
         goto error ;
      }
      else
      {
         CHAR host[ OSS_MAX_HOSTNAME + 1 ] ;
         CHAR service[ OSS_MAX_SERVICENAME + 1] ;
         rc = _route.route( id, host, service ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         rc = _frame.syncConnect( host,
                                  service,
                                  id ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         rc = _frame.syncSend( id, header, body, bodyLen ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }

   done:
      PD_TRACE_EXITRC ( SDB__NETRTAG_SYNCSND2, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_SYNCSND3, "_netRouteAgent::syncSend" )
   INT32 _netRouteAgent::syncSend( const NET_HANDLE &handle,
                                   MsgHeader *header, void *body,
                                   UINT32 bodyLen )
   {
      SDB_ASSERT( NULL != header && NULL != body,
                  "should not be NULL" ) ;

      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__NETRTAG_SYNCSND3 );

      rc = _frame.syncSend( handle, header, body, bodyLen ) ;
      if ( SDB_OK != rc )
      {
         goto error ;
      }
   done:
      PD_TRACE_EXITRC ( SDB__NETRTAG_SYNCSND3, rc );
      return rc ;
   error:
      goto done ;
   }

   // PD_TRACE_DECLARE_FUNCTION ( SDB__NETRTAG_SYNCSNDV, "_netRouteAgent::syncSendv" )
   INT32 _netRouteAgent::syncSendv( const _MsgRouteID &id,
                                    MsgHeader *header,
                                    const netIOVec &iov )
   {
      PD_TRACE_ENTRY ( SDB__NETRTAG_SYNCSNDV ) ;
      SDB_ASSERT( NULL != header, "should not be NULL" ) ;
      INT32 rc = SDB_OK ;
      rc = _frame.syncSendv( id, header, iov ) ;
      if ( SDB_OK == rc )
      {
         goto done ;
      }
      else if ( SDB_NET_NOT_CONNECT != rc )
      {
         goto error ;
      }
      else
      {
         CHAR host[ OSS_MAX_HOSTNAME + 1 ] ;
         CHAR service[ OSS_MAX_SERVICENAME + 1] ;
         rc = _route.route( id, host, service ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
         rc = _frame.syncConnect( host,
                                  service,
                                  id ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }

         rc = _frame.syncSendv( id, header, iov ) ;
         if ( SDB_OK != rc )
         {
            goto error ;
         }
      }
   done:
      PD_TRACE_EXITRC( SDB__NETRTAG_SYNCSNDV, rc ) ;
      return rc ;
   error:
      goto done ;
   }

   INT64 _netRouteAgent::netIn()
   {
      return _frame.netIn() ;
   }

   INT64 _netRouteAgent::netOut()
   {
      return _frame.netOut() ;
   }

   void _netRouteAgent::resetMon()
   {
      return _frame.resetMon() ;
   }

   /*
      _netMsgAssister implement
   */
   _netMsgAssister::_netMsgAssister()
   {
   }

   _netMsgAssister::~_netMsgAssister()
   {
   }

   void _netMsgAssister::pushMsgHandle( void * msg, NET_HANDLE handle )
   {
      ossScopedLock lock ( &_msg2NetLatch ) ;
      _mapMsg2NetHandle[msg] = handle ;
   }

   NET_HANDLE _netMsgAssister::peekMsgHandle ( void * msg )
   {
      NET_HANDLE handle = NET_INVALID_HANDLE ;
      ossScopedLock lock ( &_msg2NetLatch ) ;

      MAP_MSGNET_IT it = _mapMsg2NetHandle.find ( msg ) ;
      if ( it != _mapMsg2NetHandle.end() )
      {
         handle = it->second ;
         _mapMsg2NetHandle.erase ( it ) ;
      }

      return handle ;
   }

}

