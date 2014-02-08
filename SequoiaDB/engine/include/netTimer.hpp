/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = netTimer.hpp

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

#ifndef NETTIMER_HPP_
#define NETTIMER_HPP_
#include "core.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "oss.hpp"
#include "netDef.hpp"

namespace engine
{
   class _netTimeoutHandler : public SDBObject
   {
      public:
         virtual ~_netTimeoutHandler(){}
      public:
         virtual void handleTimeout( const UINT32 &millisec,
                                     const UINT32 &id ) = 0;
   };

   class _netTimer :
         public boost::enable_shared_from_this<_netTimer>,
         public SDBObject
   {
      public:
         _netTimer( UINT32 millisec, UINT32 id, boost::asio::io_service &io,
                    _netTimeoutHandler *handler ):
                  _timer( io, boost::posix_time::milliseconds(millisec)),
                  _handler(handler),
                  _id(id),
                  _millisec(millisec),
                  _actived(TRUE)
         {

         }

         ~_netTimer()
         {
         }
      public:
         inline void timeoutCallback( const boost::system::error_code &
                                      error )
         {
            if ( !error )
            {
               _handler->handleTimeout( _millisec, _id ) ;
            }
            asyncWait() ;
            return ;
         }
         inline UINT32 id()
         {
            return _id ;
         }
         inline UINT32 timeout()
         {
            return _millisec ;
         }
         inline void asyncWait()
         {
            if ( !_actived )
            {
               return ;
            }
            _timer.expires_from_now(boost::posix_time::milliseconds(_millisec));
            _timer.async_wait(boost::bind(&_netTimer::timeoutCallback,
                                          shared_from_this(),
                                          boost::asio::placeholders::error));
         }
         inline void cancel()
         {
            _actived = FALSE ;
            _timer.cancel() ;
         }

      private:
         boost::asio::deadline_timer _timer;
         _netTimeoutHandler *_handler ;
         UINT32 _id ;
         UINT32 _millisec ;
         BOOLEAN _actived ;
   };

   typedef boost::shared_ptr<_netTimer> NET_TH ;
}

#endif

