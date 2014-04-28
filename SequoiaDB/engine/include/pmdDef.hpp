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
   #define PMD_DFT_CONF                "sdb.conf"
   #define PMD_DFT_CAT                 "sdb.cat"
   #define PMD_OPTION_DIAG_PATH        "diaglog"
   #define PMD_OPTION_LOG_PATH         "replicalog"
   #define PMD_OPTION_BK_PATH          "bakfile"
   #define PMD_OPTION_TMPBLK_PATH      "tmp"

   #if defined (_LINUX)
   #define PROC_PATH                   "/proc"
   #define PROC_CMDLINE_PATH_FORMAT    PROC_PATH"/%s/cmdline"
   #define ENGINE_NAME                 "sequoiadb"
   #define MODIFIED_ENGINE_NAME        ENGINE_NAME "("
   #define ENGINE_NAME_PATTERN1        "sequoiadb("
   #define ENGINE_NAME_PATTERN2        ")"
   #define ENGINE_NAME_PATTERN         ENGINE_NAME_PATTERN1 "%s" ENGINE_NAME_PATTERN2
   #elif defined (_WINDOWS)
   #define ENGINE_NAME                 "sequoiadb.exe"
   #define ENGINE_NPIPE_PATTERN1       "sequoiadb"
   #define ENGINE_NPIPE_PATTERN2       "engine"
   #define ENGINE_NPIPE_PATTERN_SEP    "_"
   #define ENGINE_NPIPE_PATTERN        ENGINE_NPIPE_PATTERN1 ENGINE_NPIPE_PATTERN_SEP \
                                       ENGINE_NPIPE_PATTERN2 ENGINE_NPIPE_PATTERN_SEP \
                                       "%s"
   #endif

   #define PROC_PIPE_NAME_LEN          255
   #define ENGINE_NPIPE_MSG_PID        "$pid"
   #define LIST_TIMEOUT                0

   /*
      Config define
   */
   #define PMD_MIN_LOG_FILE_SZ                  64
   #define PMD_MAX_LOG_FILE_SZ                  2048
   #define PMD_DFT_LOG_FILE_SZ                  PMD_MIN_LOG_FILE_SZ
   #define PMD_DFT_LOG_FILE_NUM                 20

   #define PMD_REPL_PORT      1  // by default it's service port + 1
   #define PMD_SHARD_PORT     2  // by default it's service port + 2
   #define PMD_CAT_PORT       3  // by default it's service port + 3
   #define PMD_REST_PORT      4  // by default it's service port + 4

   /*
      PMD Option define
   */
   #define PMD_ADD_PARAM_OPTIONS_BEGIN( desc )  desc.add_options()
   #define PMD_ADD_PARAM_OPTIONS_END ;
   #define PMD_COMMANDS_STRING( a, b )       (string(a) +string( b)).c_str()

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

