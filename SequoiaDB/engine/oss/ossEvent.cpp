/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = ossEvent.cpp

   Descriptive Name = Data Management Service Header

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/12/2012  Xu Jianhui  Initial Draft

   Last Changed =

*******************************************************************************/

#include "ossEvent.hpp"
#include "pdTrace.hpp"
#include "ossTrace.hpp"

namespace engine
{
   _ossEvent::_ossEvent ()
   {
      _signal = 0 ;
      _waitNum = 0 ;
      _useData = 0 ;
   }

   _ossEvent::~_ossEvent ()
   {
      _signal = 0 ;
   }

   UINT32 _ossEvent::waitNum ()
   {
      boost::mutex::scoped_lock lock ( _mutex ) ;
      return _waitNum ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__OSSEVN_WAIT, "_ossEvent::wait" )
   INT32 _ossEvent::wait ( INT64 millisec, INT32 *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB__OSSEVN_WAIT );
      boost::system_time const timeout = boost::get_system_time() + 
            boost::posix_time::milliseconds (millisec) ;

      boost::mutex::scoped_lock lock ( _mutex ) ;

      ++_waitNum ;
      if ( !_signal )
      {
         if ( millisec < 0 )
         {
            _cond.wait ( lock ) ;
         }
         else if ( !_cond.timed_wait ( lock, timeout ) )
         {
            --_waitNum ;
            rc = SDB_TIMEOUT ;
            goto done ;
         }
      }
      if ( pData )
      {
         *pData = _useData ;
      }
      _onWait () ;

      --_waitNum ;
   done :
      PD_TRACE_EXITRC ( SDB__OSSEVN_WAIT, rc );
      return rc ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__OSSEN_SIGNAL, "_ossEvent::signal" )
   INT32 _ossEvent::signal ( INT32 data )
   {
      PD_TRACE_ENTRY ( SDB__OSSEN_SIGNAL );
      boost::mutex::scoped_lock lock ( _mutex ) ;
      _signal = 1 ;
      _useData = data ;
      //lock.unlock () ;
      _cond.notify_one () ;

      PD_TRACE_EXIT ( SDB__OSSEN_SIGNAL );
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__OSSEN_SIGALL, "_ossEvent::signalAll" )
   INT32 _ossEvent::signalAll ( INT32 data )
   {
      PD_TRACE_ENTRY ( SDB__OSSEN_SIGALL );
      boost::mutex::scoped_lock lock ( _mutex ) ;
      _signal = 1 ;
      _useData = data ;
      //lock.unlock () ;
      _cond.notify_all () ;

      PD_TRACE_EXIT ( SDB__OSSEN_SIGALL );
      return SDB_OK ;
   }

   PD_TRACE_DECLARE_FUNCTION ( SDB__OSSVN_RESET, "_ossEvent::reset" )
   INT32 _ossEvent::reset ()
   {
      PD_TRACE_ENTRY ( SDB__OSSVN_RESET );
      boost::mutex::scoped_lock lock ( _mutex ) ;
      _signal = 0 ;

      PD_TRACE_EXIT ( SDB__OSSVN_RESET );
      return SDB_OK ;
   }

   void _ossEvent::_onWait ()
   {
   }

   _ossAutoEvent::_ossAutoEvent ()
   {
   }

   _ossAutoEvent::~_ossAutoEvent ()
   {
   }

   void _ossAutoEvent::_onWait ()
   {
      _signal = 0 ;
   }
}
