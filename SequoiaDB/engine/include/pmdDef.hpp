/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdDef.hpp

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
#ifndef PMD_DEF_HPP__
#define PMD_DEF_HPP__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{
   /*
      pmdEDUEventTypes define
   */
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
      PMD_EDU_EVENT_BP_RESUME,    // break point resume
      PMD_EDU_EVENT_TRANS_STOP    // stop transaction
   } ;

   /*
      PMD_EVENT_MESSAGES define, for different event
   */
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

   /*
      _pmdEDUEvent define
   */
   class _pmdEDUEvent : public SDBObject
   {
   public :
      pmdEDUEventTypes  _eventType ;
      UINT64            _userData ;
      BOOLEAN           _release ;  // can release flag
      void              *_Data ;

      _pmdEDUEvent ( pmdEDUEventTypes type = PMD_EDU_EVENT_NONE,
                     BOOLEAN release = FALSE,
                     void *data = NULL,
                     UINT64 userData = 0 )
      {
         _reset ( type, release, data, userData ) ;
      }
      _pmdEDUEvent( const _pmdEDUEvent &rhs )
      {
         _reset ( rhs._eventType, rhs._release, rhs._Data, rhs._userData ) ;
      }
      _pmdEDUEvent& operator=( const _pmdEDUEvent &rhs )
      {
         _reset( rhs._eventType, rhs._release, rhs._Data, rhs._userData ) ;
         return *this ;
      }
      void reset ()
      {
         _reset () ;
      }

   protected:
      void _reset ( pmdEDUEventTypes type = PMD_EDU_EVENT_NONE,
                    BOOLEAN release = FALSE,
                    void *data = NULL,
                    UINT64 userData = 0 )
      {
         _eventType  = type ;
         _release    = release ;
         _Data       = data ;
         _userData   = userData ;
      }

   } ;

   typedef class _pmdEDUEvent pmdEDUEvent ;

   /*
      define
   */
   #define PMD_DFT_CONF          "sdb.conf"
   #define PMD_DFT_CAT           "sdb.cat"
   #define PMD_DFT_DIAGLOG       "sdbdiag.log"

   /*
      PDM_SESSION_TYPE define
   */
   enum PDM_SESSION_TYPE
   {
      PMD_SESSION_LOCAL                = 1,
      PMD_SESSION_COORD,
      PMD_SESSION_REST
   } ;

}

#endif // PMD_DEF_HPP__

