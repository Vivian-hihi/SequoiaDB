/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdEDUEvent.hpp

   Descriptive Name = Process MoDel Engine Dispatchable Unit Event Header

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains structure for events that
   used as inter-EDU communications.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDEDUEVENT_HPP__
#define PMDEDUEVENT_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "ossQueue.hpp"

namespace engine
{
   enum pmdEDUEventTypes
   {
      PMD_EDU_EVENT_NONE = 0 ,
      PMD_EDU_EVENT_TERM ,        // terminate EDU
      PMD_EDU_EVENT_RESUME,       // resume a EDU, the data is startEDU's argv
      PMD_EDU_EVENT_ACTIVE,
      PMD_EDU_EVENT_DEACTIVE,
      PMD_EDU_EVENT_MSG,          // pmd msg event
      PMD_EDU_EVENT_TIMEOUT,      // pmd edu timeout,
      PMD_EDU_EVENT_LOCKWAKEUP,   // transaction-lock wake up
      PMD_EDU_EVENT_TRACE_BREAKPOINT_RESUME,
      PMD_EDU_EVENT_TRANS_STOP    // stop transaction
   } ;

   // event message for different type of messages
   union PMD_EVENT_MESSAGES
   {
      // for PMD_EDU_EVENT_TIMEOUT
      struct timeoutMsg
      {
         UINT64   timerID ;
         UINT32   interval ;
         UINT64   occurTime ;
      } timeoutMsg ;
   } ;

   typedef union PMD_EVENT_MESSAGES PMD_EVENT_MESSAGES ;

   class _pmdEDUEvent : public SDBObject
   {
   public :
      pmdEDUEventTypes _eventType ;
      BOOLEAN          _release ;  // can release flag
      void *_Data ;
      _pmdEDUEvent ()
      {
         reset () ;
      }
      _pmdEDUEvent ( pmdEDUEventTypes type )
      {
         _eventType = type ;
         _release = FALSE ;
         _Data = NULL ;
      }
      _pmdEDUEvent ( pmdEDUEventTypes type, BOOLEAN release, void *data )
      {
         _eventType = type ;
         _release = release ;
         _Data = data ;
      }
      _pmdEDUEvent& operator=(const _pmdEDUEvent &rhs)
      {
         _eventType = rhs._eventType ;
         _release = rhs._release ;
         _Data = rhs._Data ;
         return *this ;
      }
      void reset ()
      {
         _eventType = PMD_EDU_EVENT_NONE ;
         _release = FALSE ;
         _Data = NULL ;
      }
   } ;
   typedef class _pmdEDUEvent pmdEDUEvent ;
}
#endif
