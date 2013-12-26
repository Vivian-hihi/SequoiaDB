/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = pmdProc.cpp

   Descriptive Name = pmdProc

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/26/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#include "pmdProc.hpp"
#include "ossEDU.hpp"
#include "pd.hpp"

#if defined (_LINUX)
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace engine
{
   BOOLEAN iPmdProc::_isRunning = FALSE;

   iPmdProc::iPmdProc()
   {
      _isRunning = TRUE;
      regSignalHandler();
   }

   iPmdProc::~iPmdProc()
   {
   }

   void iPmdProc::stop()
   {
      _isRunning = FALSE;
   }

   INT32 iPmdProc::regSignalHandler()
   {
      INT32 rc = SDB_OK;
#if defined (_LINUX)
      ossSigSet sigSet ;
      sigSet.sigAdd( SIGHUP );
      sigSet.sigAdd( SIGINT );
      sigSet.sigAdd( SIGTERM );
      sigSet.sigAdd( SIGPWR );
      rc = ossRegisterSignalHandle( sigSet,
               (SIG_HANDLE)(&iPmdProc::stop) );
      PD_RC_CHECK( rc, PDERROR,
                  "failed to register signals, rc = %d",
                  rc );
   done:
      return rc;
   error:
      goto done;
#else
      return rc;
#endif
   }

   BOOLEAN iPmdProc::isRunning()
   {
      return _isRunning;
   }
}