/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossQueue.hpp

   Descriptive Name = Operating System Services Queue Header

   When/how to use: this program may be used on binary and text-formatted
   versions of OSS component. This file contains structure for thread-safe
   queue, including push, try_pop, wait_and_pop and timed_wait_and_pop
   operations.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef OSSQUEUE_HPP__
#define OSSQUEUE_HPP__
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/thread_time.hpp>
#include "core.hpp"
#include "oss.hpp"
template<typename Data>
class ossQueue : public SDBObject
{
private :
   std::queue<Data> _queue ;
   mutable boost::mutex _mutex ;
   boost::condition_variable _cond ;
public :
   UINT32 size ()
   {
      boost::mutex::scoped_lock lock ( _mutex ) ;
      return (UINT32)_queue.size() ;
   }
   void push ( Data const& data )
   {
      boost::mutex::scoped_lock lock ( _mutex ) ;
      _queue.push ( data ) ;
      lock.unlock () ;
      _cond.notify_one () ;
   }

   BOOLEAN empty() const
   {
      boost::mutex::scoped_lock lock ( _mutex ) ;
      return _queue.empty () ;
   }

   BOOLEAN try_pop ( Data& value )
   {
      boost::mutex::scoped_lock lock ( _mutex ) ;
      if ( _queue.empty () )
         return FALSE ;
      value = _queue.front () ;
      _queue.pop () ;
      return TRUE ;
   }

   void wait_and_pop ( Data& value )
   {
      boost::mutex::scoped_lock lock ( _mutex ) ;
      while ( _queue.empty () )
         _cond.wait ( lock ) ;
      value = _queue.front () ;
      _queue.pop () ;
   }

   BOOLEAN timed_wait_and_pop ( Data& value, INT64 millsec )
   {
      boost::system_time const timeout=boost::get_system_time() + 
            boost::posix_time::milliseconds(millsec);
      boost::mutex::scoped_lock lock ( _mutex ) ;
      // if timed_wait return false, that means we failed by timeout
      while ( _queue.empty () )
         if ( !_cond.timed_wait ( lock, timeout ) )
            return FALSE ;
      value = _queue.front () ;
      _queue.pop () ;
      return TRUE ;
   }
} ;
//typedef class ossQueue ossQueue ;

#endif
