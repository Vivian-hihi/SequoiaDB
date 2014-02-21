/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = rtnCM.cpp

   Descriptive Name = rtnClusterManager

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          12/17/2013  JHL  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef RTNCM_HPP__
#define RTNCM_HPP__

#include "pmdOptions.h"
#include "pmdDaemon.hpp"

namespace CLSMGR
{

#define SDBCM_OPTION_PREFIX         "--"
#define SDBCM_DFT_PORT              11790

// sdbcm configure file
#define SDBCM_CONF_PATH             ".." OSS_FILE_SEP "conf" OSS_FILE_SEP "sdbcm.conf"
#define SDBCM_CONF_DFTPORT          "defaultPort"
#define SDBCM_CONF_PORT             "_Port"
#define SDBCM_RESTART_COUNT         "RestartCount"
#define SDBCM_RESTART_INTERVAL      "RestartInterval"       // minute
#define SDBCM_AUTO_START            "AutoStart"

#define PMD_DFT_CONF_PATH           ".." OSS_FILE_SEP "conf" OSS_FILE_SEP "local"

// remote operation code
#define SDBSTART              1
#define SDBSTOP               2
#define SDBADD                3
#define SDBMODIFY             4
#define SDBRM                 5

// stop status
#define STOPFAIL              1
#define STOPPART              3
#define SDBCM_EXE_FILE_NAME            "sdbcm"
#define SDBCM_LOG_DIR                  ".."OSS_FILE_SEP"conf"OSS_FILE_SEP"log"

   class cCMService : public engine::iPmdDMNChildProc
   {
   public:
      cCMService();
      ~cCMService(){}
      INT32 init();
   private:
      virtual const CHAR *getProgramName();
      const CHAR *getArguments();
      virtual INT32 svcMain( INT32 argc, CHAR **argv );
   };
}

#endif
