/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netRouteAgent.hpp

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

#ifndef NETROUTEAGENT_HPP_
#define NETROUTEAGENT_HPP_

#include "core.hpp"
#include "oss.hpp"
#include "netRoute.hpp"
#include "netFrame.hpp"

namespace engine
{
   class _netRouteAgent : public SDBObject
   {
      public:
         _netRouteAgent( _netMsgHandler *handler ) ;

      public:
         inline void run()
         {
            return _frame.run() ;
         }

         inline void stop()
         {
            return _frame.stop() ;
         }

         inline void setLocalID( const _MsgRouteID &id )
         {
            _frame.setLocal( id ) ;
            _route.setLocal( id ) ;
            return ;
         }

         inline MsgRouteID localID()
         {
            return _route.local() ;
         }

         inline INT32 addTimer( UINT32 millsec,
                                _netTimeoutHandler *handler,
                                UINT32 &timerid )
         {
            return _frame.addTimer( millsec, handler, timerid ) ;
         }

         inline INT32 removeTimer( UINT32 timerid )
         {
            return _frame.removeTimer( timerid ) ;
         }

         inline void close( const _MsgRouteID &id )
         {
            _frame.close( id ) ;
         }

         inline void close( const NET_HANDLE &handle )
         {
            _frame.close( handle ) ;
         }

         inline void disconnectAll()
         {
            _frame.close() ;
         }

         inline  io_service *ioservice()
         {
            return &( _frame.ioservice() ) ;
         }

         inline INT32 route( const _MsgRouteID &id,
                             _netRouteNode &node )
         {
            return _route.route( id, node ) ;
         }

      public:
         INT32 listen( const _MsgRouteID &id ) ;

         INT32 syncSend( const _MsgRouteID &id,
                         void *header ) ;

         INT32 syncSend( const NET_HANDLE &handle,
                         void *header ) ;

         INT32 syncSend( const _MsgRouteID &id,
                         MsgHeader *header,
                         void *body,
                         UINT32 bodyLen ) ;

         INT32 syncSend( const NET_HANDLE &handle,
                         MsgHeader *header,
                         void *body,
                         UINT32 bodyLen ) ;

         INT32 syncSendv( const _MsgRouteID &id,
                          MsgHeader *header,
                          const netIOVec &iov ) ;

         INT32 updateRoute( const _MsgRouteID &id,
                           const CHAR *host,
                           const CHAR *service ) ;

         INT32 updateRoute( const _MsgRouteID &id,
                            const _netRouteNode &node ) ;

         INT32 updateRoute( const _MsgRouteID &oldID,
                            const _MsgRouteID &newID ) ;

         INT64 netIn() ;

         INT64 netOut() ;

         void resetMon() ;

      private:
         _netFrame _frame ;
         _netRoute _route ;
   } ;

   typedef class _netRouteAgent netRouteAgent ;
}

#endif

