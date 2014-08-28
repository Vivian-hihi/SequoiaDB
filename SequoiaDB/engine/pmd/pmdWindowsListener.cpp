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

   Source File Name = pmdWindowsListener.cpp

   Descriptive Name = Process MoDel Windows Listener

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains entry point for local listener
   that only avaliable on Windows.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          09/14/2012  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#if defined (_WINDOWS)
#include <stdio.h>
#include "pd.hpp"
#include "pmd.hpp"
#include "pmdEDUMgr.hpp"
#include "ossUtil.hpp"
#include "ossNPipe.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

namespace engine
{
   // 1 seconds timeout
   #define PMD_WL_NPIPE_TIMEOUT        1
   #define PMD_WL_NPIPE_BUFSZ          1024

   // PD_TRACE_DECLARE_FUNCTION ( SDB_PMDWINLSTNNPNTPNT, "pmdWindowsListenerEntryPoint" )
   INT32 pmdWindowsListenerEntryPoint ( pmdEDUCB *cb, void *pData )
   {
      INT32 rc = SDB_OK ;
      PD_TRACE_ENTRY ( SDB_PMDWINLSTNNPNTPNT );
      EDUID myEDUID = cb->getID () ;
      CHAR namedPipe [ OSS_NPIPE_MAX_NAME_LEN + 1 ] = {0} ;
      OSSNPIPE pipeHandle ;
      BOOLEAN pipeCreated = FALSE ;
      pmdEDUMgr * eduMgr = cb->getEDUMgr() ;
      CHAR tempBuffer [ PMD_WL_NPIPE_BUFSZ ] = {0} ;
      const CHAR *pSvcName = ( const CHAR* )pData ;

      INT32 dataSize = 0 ;
      INT64 readSize = 0 ;
      INT32 len = 0 ;
      rc = eduMgr->activateEDU ( myEDUID ) ;
      if ( rc )
      {
         PD_LOG ( PDERROR, "Failed to activate EDU, rc: %d", rc ) ;
         goto error ;
      }
      ossSnprintf ( namedPipe, OSS_NPIPE_MAX_NAME_LEN,
                    ENGINE_NPIPE_PREFIX"%s", pSvcName ) ;
      PD_LOG ( PDINFO, "Attempt to create named pipe: %s",
               namedPipe ) ;

      // create a named pipe
      rc = ossCreateNamedPipe ( namedPipe, PMD_WL_NPIPE_BUFSZ,
                                PMD_WL_NPIPE_BUFSZ,
                                OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK_WITH_TIMEOUT,
                                OSS_NPIPE_UNLIMITED_INSTANCES,
                                PMD_WL_NPIPE_TIMEOUT,
                                pipeHandle ) ;
      if ( rc )
      {
         // if we are not able to create named pipe, then we are not able
         // to stop it using sdbstop.exe. So we should nicely shutdown
         // database in order to prevent killing process later
         PD_LOG ( PDSEVERE, "Failed to create named pipe: %s, rc = %d",
                  namedPipe, rc ) ;
         goto error ;
      }

      pipeCreated = TRUE ;

      // just sit here do nothing at the moment
      while ( !cb->isDisconnected() )
      {
         rc = ossConnectNamedPipe ( pipeHandle,
                                    OSS_NPIPE_DUPLEX | OSS_NPIPE_BLOCK,
                                    PMD_WL_NPIPE_TIMEOUT ) ;
         if ( rc )
         {
            // we just loop if nothing returns in PMD_WL_NPIPE_TIMEOUT
            if ( SDB_TIMEOUT == rc )
            {
               continue ;
            }
            // if we are not able to connect named pipe, then we are not able
            // to stop it using sdbstop.exe. So we should nicely shutdown
            // database in order to prevent killing process later
            PD_LOG ( PDSEVERE, "Failed to connect named pipe: %s, rc = %d",
                     namedPipe, rc ) ;
            goto error ;
         }
         readSize = 0 ;
         while ( 0 == readSize && !cb->isDisconnected() )
         {
            // then let's read from pipe. For this version let's just read
            rc = ossReadNamedPipe ( pipeHandle, tempBuffer, PMD_WL_NPIPE_BUFSZ,
                                    &readSize, PMD_WL_NPIPE_TIMEOUT ) ;
            if ( rc )
            {
               // if we simply timeout, maybe the sender is too slow. Let's continue
               if ( SDB_TIMEOUT == rc )
                  continue ;
               // if we failed to read, let's dump error and break out the loop
               PD_LOG ( PDERROR, "Failed to read packet, rc = %d", rc ) ;
               readSize = 0 ;
               rc = SDB_OK ;
               break ;
            }
         }

         if ( readSize > 0 )
         {
            PD_LOG ( PDINFO, "Received message from windows listener: %s",
                     tempBuffer ) ;
            if ( ossStrncmp ( tempBuffer, ENGINE_NPIPE_MSG_SHUTDOWN,
                              sizeof(ENGINE_NPIPE_MSG_SHUTDOWN) ) == 0 )
            {
               PD_LOG ( PDEVENT, "Shutdown message is received" ) ;
               PMD_SHUTDOWN_DB( SDB_OK ) ;
            }
            else if ( ossStrncmp ( tempBuffer, ENGINE_NPIPE_MSG_PID,
                                   sizeof(ENGINE_NPIPE_MSG_PID) ) == 0 )
            {
               INT64 writeSize = 0 ;
               OSSPID currentProcessPID = ossGetCurrentProcessID () ;
               rc = ossWriteNamedPipe ( pipeHandle, (CHAR*)&currentProcessPID,
                                        sizeof(currentProcessPID),
                                        &writeSize ) ;
               if ( rc )
               {
                  PD_LOG ( PDWARNING, "Failed to write pid to named pipe, "
                           "rc = %d", rc ) ;
               }
            }
            else if ( 0 == ossStrncmp( tempBuffer, ENGINE_NPIPE_MSG_TYPE,
                                       sizeof( ENGINE_NPIPE_MSG_TYPE ) ) )
            {
               INT64 writeSize = 0 ;
               INT32 type = pmdGetDBType() ;
               rc = ossWriteNamedPipe ( pipeHandle, (CHAR*)&type,
                                        sizeof(type), &writeSize ) ;
               if ( rc )
               {
                  PD_LOG ( PDWARNING, "Failed to write type to named pipe, "
                           "rc = %d", rc ) ;
               }
            }
         }
         ossDisconnectNamedPipe ( pipeHandle ) ;
      }

   done :
      if ( pipeCreated )
      {
         ossDeleteNamedPipe ( pipeHandle ) ;
      }
      PD_TRACE_EXITRC ( SDB_PMDWINLSTNNPNTPNT, rc );
      return rc;
   error :
      switch ( rc )
      {
      case SDB_SYS :
         PD_LOG ( PDSEVERE, "System error occured" ) ;
         break ;
      default :
         PD_LOG ( PDSEVERE, "Internal error" ) ;
      }
      PD_LOG ( PDSEVERE, "Shutdown database" ) ;
      PMD_SHUTDOWN_DB( rc ) ;
      goto done ;
   }
}

#endif // _WINDOWS

