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
#define SDBCM_EXE_FILE_NAME         "sdbcm"
#define SDBCM_CONF_DIR_NAME         "conf"
#define SDBCM_LOCAL_DIR_NAME        "local"
#define SDBCM_LOG_DIR_NAME          "log"
#define SDBCM_CFG_FILE_NAME         "sdbcm.conf"
#define SDB_CM_ROOT_PATH            ".." OSS_FILE_SEP SDBCM_CONF_DIR_NAME OSS_FILE_SEP

#define SDBCM_CONF_PATH_FILE        SDB_CM_ROOT_PATH SDBCM_CFG_FILE_NAME
#define SDBCM_LOCAL_PATH            SDB_CM_ROOT_PATH SDBCM_LOCAL_DIR_NAME
#define SDBCM_LOG_PATH              SDB_CM_ROOT_PATH SDBCM_LOG_DIR_NAME

#define SDBCM_CONF_DFTPORT          "defaultPort"
#define SDBCM_CONF_PORT             "_Port"
#define SDBCM_RESTART_COUNT         "RestartCount"
#define SDBCM_RESTART_INTERVAL      "RestartInterval"       // minute
#define SDBCM_AUTO_START            "AutoStart"

// remote operation code
#define SDBSTART              1
#define SDBSTOP               2
#define SDBADD                3
#define SDBMODIFY             4
#define SDBRM                 5

// stop status
#define STOPFAIL              1
#define STOPPART              3

   /*
      cCMService define
   */
   class cCMService : public engine::iPmdDMNChildProc
   {
   public:
      cCMService() ;
      ~cCMService(){}
      INT32 init() ;

      virtual const CHAR *getProgramName() ;

   private:
      const CHAR *getArguments();
      virtual INT32 svcMain( INT32 argc, CHAR **argv );
   } ;

}

#endif // RTNCM_HPP__

