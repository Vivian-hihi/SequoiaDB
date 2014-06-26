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

   Source File Name = pmdDaemon.hpp

   Descriptive Name = pmdDaemon

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          10/09/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef PMDDAEMON_HPP__
#define PMDDAEMON_HPP__

#include "ossFeat.h"
#include "ossTypes.h"
#include "ossShMem.hpp"
#include "oss.h"
#include "pmdProc.hpp"
#include "ossEvent.hpp"
#include "ossLatch.hpp"

namespace engine
{
#define PMDDMN_INTERVAL_TIME              (1*1000)   //1 second
#define PMDDMN_INTERVAL_TIME_DMN          (PMDDMN_INTERVAL_TIME * 2)
#define PMDDMN_LOG_SUFFIX                 ".log"
#define PMDDMN_SVCNAME_DEFAULT            "sdbcmd"
#define PMDDMN_SHM_TAG                    "sequoiadbDMN"
#define PMDDMN_SHMSTAT_EXPRIRED_TIMES     10
#define PMDDMN_STOP_CHILD_MAX_TRY_TIMES   60
#define PMDDMN_STOP_CHILD_WAIT_TIME       (5*1000)

#define PMDDMN_STOP_WAIT_TIME             (60*1000)

#if defined (_LINUX)
   #define PMDDMN_SHMKEY_DEFAULT          50010
#elif defined (_WINDOWS)
   #define PMDDMN_SHMKEY_DEFAULT          "sequoiadbDMN"
#endif

   enum pmdDMNSHMStat
   {
      PMDDMN_SHM_STAT_DAEMON = 0,         // the daemon-process will modify the buffer
      PMDDMN_SHM_STAT_CHILDREN = 1        // the children-process will modify the buffer
   };

   enum pmdDMNSHMCmd
   {
      PMDDMN_SHM_CMD_INVALID     = 0,

      // daemon command:
      PMDDMN_SHM_CMD_DMN_BEGIN   = 1,
      PMDDMN_SHM_CMD_DMN_QUIT    = 2,

      PMDDMN_SHM_CMD_DMN_END     = 0x00FF,

      // children command:
      PMDDMN_SHM_CMD_CHL_BEGIN   = 0x0100,
      PMDDMN_SHM_CMD_CHL_QUIT    = 0x0200,

      PMDDMN_SHM_CMD_CHL_END     = 0xFF00
   };
   typedef struct _pmdDMNProcInfo
   {
#define PMDDMN_SHM_CMD_DMN_MASK        0x00FF
#define PMDDMN_SHM_CMD_CHL_MASK        0xFF00
      CHAR           szTag[32];
      OSSPID         pid;
      pmdDMNSHMStat  stat;
      INT32          cmd;
      INT32          exitCode;
      UINT32         sn;
   public:
      _pmdDMNProcInfo();
      BOOLEAN isInit();
      void init();
      INT32 setDMNCMD( pmdDMNSHMCmd newCMD );
      INT32 setCHLCMD( pmdDMNSHMCmd newCMD );
      INT32 getDMNCMD();
      INT32 getCHLCMD();
   }pmdDMNProcInfo;

   class iPmdDMNChildProc : public iPmdProc
   {
   private:
      virtual const CHAR *getProgramName() = 0;
      virtual const CHAR *getArguments(){ return NULL; }
      virtual INT32 svcMain( INT32 argc, CHAR **argv ) = 0;

   public:
      iPmdDMNChildProc();
      virtual ~iPmdDMNChildProc();
      virtual INT32 init( ossSHMKey shmKey ) ;
      BOOLEAN isChildRunning();
      INT32 DMNProcessCMD( INT32 cmd );
      INT32 ChildProcessCMD( INT32 cmd );
      INT32 start();    // call by daemon
      INT32 stop();     // call by daemon
      INT32 run( INT32 argc, CHAR **argv ); // call in main()
      INT32 active();
      void  deactive();

      void  lock() ;
      void  unlock() ;
      void  signal() ;
      INT32 wait( INT64 timeout = PMDDMN_STOP_WAIT_TIME ) ;

   private:
      INT32 allocSHM( ossSHMKey shmKey );
      INT32 allocSHM();
      INT32 attachSHM( ossSHMKey shmKey );
      INT32 attachSHM();
      void detachSHM();
      void freeSHM();
      INT32 startSyncThr();
      void syncProcesserInfo();
      virtual const CHAR *getExecuteFile();

   private:
      pmdDMNProcInfo       *_procInfo;
      ossSHMMid            _shmMid;
      UINT32               _deadTime;
      CHAR                 _execName[ OSS_MAX_PATHSIZE + 1 ];
      ossSHMKey            _shmKey;
      BOOLEAN              _syncExit;
      OSSPID               _pid;
#if defined ( _WINDOWS )
      ossSpinXLatch        _mutex ;
      ossEvent             _event ;
#endif // _WINDOWS

   };

   class cPmdDaemon : public iPmdProc
   {
   public:
      cPmdDaemon( const CHAR *pDMNSvcName );
      virtual ~cPmdDaemon();
      INT32 run( INT32 argc, CHAR **argv );
      INT32 addChildrenProcess( iPmdDMNChildProc *childProc );
      void  stop();

   public:
      INT32 init();

   private:
      cPmdDaemon(){};
      static INT32 _run( INT32 argc, CHAR **argv );

   private:
      CHAR                       _procName[OSS_MAX_PATHSIZE + 1];
      static iPmdDMNChildProc    *_process;
   };

}

#endif // PMDDAEMON_HPP__

