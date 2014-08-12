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

   Source File Name = omagentDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          07/29/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_DEF_HPP__
#define OMAGENT_DEF_HPP__

#include "core.hpp"
#include "oss.hpp"

namespace engine
{

   /*
      [ sdbcm.conf ] Config Param Define
   */
   #define SDBCM_CONF_DFTPORT          "defaultPort"
   #define SDBCM_CONF_PORT             "_Port"
   #define SDBCM_RESTART_COUNT         "RestartCount"
   #define SDBCM_RESTART_INTERVAL      "RestartInterval"       // minute
   #define SDBCM_AUTO_START            "AutoStart"
   #define SDBCM_DIALOG_LEVEL          "DiagLevel"

   #define SDBCM_DFT_PORT              11790
   #define SDBCM_OPTION_PREFIX         "--"

   /*
      CM REMOTE CODE DEFINE
   */
   enum CM_REMOTE_OP_CODE
   {
      SDBSTART       = 1,
      SDBSTOP        = 2,
      SDBADD         = 3,
      SDBMODIFY      = 4,
      SDBRM          = 5,
      SDBSTARTALL    = 6,
      SDBSTOPALL     = 7
   } ;

   /*
      SDB STOP RETURN RC
   */
   #define STOPFAIL              1
   #define STOPPART              3

   /*
      cm define
   */
   #define SDBCM_CONF_DIR_NAME         "conf"
   #define SDBCM_LOCAL_DIR_NAME        "local"
   #define SDBCM_LOG_DIR_NAME          "log"
   #define SDBOMA_SCRIPT_DIR_NAME      "script"

   #define SDBCM_EXE_FILE_NAME         "sdbcm"
   #define SDBCM_CFG_FILE_NAME         SDBCM_EXE_FILE_NAME".conf"
   #define SDBCM_DIALOG_FILE_NAME      SDBCM_EXE_FILE_NAME".log"

   #define SDB_CM_ROOT_PATH            ".." OSS_FILE_SEP SDBCM_CONF_DIR_NAME OSS_FILE_SEP
   #define SDBCM_CONF_PATH_FILE        SDB_CM_ROOT_PATH SDBCM_CFG_FILE_NAME
   #define SDBCM_LOCAL_PATH            SDB_CM_ROOT_PATH SDBCM_LOCAL_DIR_NAME
   #define SDBCM_LOG_PATH              SDB_CM_ROOT_PATH SDBCM_LOG_DIR_NAME
   #define SDBOMA_SCRIPT_PATH          SDB_CM_ROOT_PATH SDBOMA_SCRIPT_DIR_NAME

#if defined (_LINUX)
      #define SDBCM_NAME_PATTERN       "sdbcm(%s)"
      #define SDBSTARTPROG             "sdbstart"
      #define SDBSTOPPROG              "sdbstop"
#elif defined (_WINDOWS)
      #define SDBSTARTPROG             "sdbstart.exe"
      #define SDBSTOPPROG              "sdbstop.exe"
#endif


}

#endif // OMAGENT_DEF_HPP__

