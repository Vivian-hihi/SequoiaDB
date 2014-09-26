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
      #define SDBSTARTPROG             "sdbstart"
      #define SDBSTOPPROG              "sdbstop"
#elif defined (_WINDOWS)
      #define SDBSTARTPROG             "sdbstart.exe"
      #define SDBSTOPPROG              "sdbstop.exe"
#endif

   #define SDB_OMA_USER                "OMA_ADMIN"
   #define SDB_OMA_USERPASSWD          "OMA_ADMIN_PASSWD"

   /*
      oma command define
   */
   #define OMA_CMD_SCAN_HOST                          OM_SCAN_HOST_REQ
   #define OMA_CMD_BASIE_CHECK_HOST                   OM_BASIC_CHECK_REQ
   #define OMA_CMD_INSTALL_REMOTE_AGENT               OM_INSTALL_REMOTE_AGENT
   #define OMA_CMD_CHECK_HOST                         OM_CHECK_HOST_REQ
   #define OMA_CMD_UNINSTALL_REMOTE_AGENT             OM_UNINSTALL_REMOTE_AGENT
   #define OMA_CMD_ADD_HOST                           OM_ADD_HOST_REQ
   #define OMA_CMD_INSTALL_DB_BUSINESS                OM_INSTALL_BUSINESS_REQ
   #define OMA_CMD_QUERY_INSTALL_DB_BUSINESS_PROGRESS OM_QUERY_PROGRESS
   //#define OMA_CMD_ROLLBACK_INSTALL_DB_BUSINESS       OM_ROLLBACK_TRANSACTION_REQ
   #define OMA_CMD_UPDATE_HOSTS                       OM_UPDATE_HOSTNAME_REQ

   /*
      oma internal command
   */
   #define OMA_CMD_CRRATE_VIRTUAL_COORD                   "create virtual coord"
   #define OMA_CMD_REMOVE_VIRTUAL_COORD                   "remove virtual coord"
   #define OMA_CMD_ROLLBACK_ADD_HOSTS                     "rollback add hosts"
   #define OMA_CMD_RUN_CREATE_COORD                   "run create coord job"
   #define OMA_CMD_RUN_CREATE_CATALOG                 "run create catalog job"
   #define OMA_CMD_RUN_CREATE_DATANODE                "run create data node job"
   
   /*
      oma job
   */
   #define OMA_JOB_CREATE_CATALOG                     "create catalog job"
   #define OMA_JOB_CREATE_COORD                       "create coord job"
   #define OMA_JOB_START_INSTALL_DB_BUSINESS          "start install db business job"
   #define OMA_JOB_ROLLBACK_INSTALL_DB_BUSINESS       "rollback install db business job"
   #define OMA_JOB_ROLLBACK_CATALOG                   "rollback create catalog job"
   #define OMA_JOB_ROLLBACK_COORD                     "rollback create coord job"
   #define OMA_JOB_REMOVE_VIRTUAL_COORD               "remove virtual coord job"

   /*
      oma js file
   */
   #define FILE_DEFINE                      "define.js"
   #define FILE_ERROR                       "error.js"
   #define FILE_COMMON                      "common.js"
   #define FILE_FUNC                        "func.js"
   
   #define FILE_SCAN_HOST                   "scanHost.js"
   #define FILE_BASIC_CHECK_HOST            "basicCheckHost.js"
   #define FILE_INSTALL_REMOTE_AGENT        "installRemoteAgent.js"
   #define FILE_CHECK_HOST                  "checkHost.js"
   #define FILE_CHECK_HOST_ITEM             "checkHostItem.js"
   #define FILE_UNINSTALL_REMOTE_AGENT      "uninstallRemoteAgent.js"
   #define FILE_ADD_HOST                    "addHost.js"
   
   #define FILE_GET_REMOTE_AGENT_STATUS     "getRemoteAgentStatus.js"
   #define FILE_CREATE_VIRTUAL_COORD        "createVirtualCoord.js"
   #define FILE_REMOVE_VIRTUAL_COORD        "removeVirtualCoord.js"
   #define FILE_GET_PORT_STATUS             "getPortStatus.js"
   #define FILE_REG_HOSTS_INFO              "regHostsInfo.js"
   #define FILE_GET_HOST_NAME               "getHostName.js"
   #define FILE_ADDHOST_ROLLBACK_INTERNAL   "addHostRollbackInternal.js"
   #define FILE_UPDATE_HOSTS_INFO           "updateHostsInfo.js"


   #define FILE_CREATE_CATALOG              "createCatalog.js"
   #define FILE_CREATE_COORD                "createCoord.js"
   #define FILE_CREATE_DATANODE             "createData.js"
   
   #define FILE_REMOVE_CATALOG              "removeCatalog.js"
   #define FILE_REMOVE_COORD                "removeCoord.js"
   #define FILE_REMOVE_DATANODE             "removeData.js"
   
   #define FILE_ROLLBACK_CATALOG            "rollbackCatalog.js"
   #define FILE_ROLLBACK_COORD              "rollbackCoord.js"
   #define FILE_ROLLBACK_DATANODE           "rollbackDataNode.js"

   /*
      oma js argument type
   */
   #define JS_ARG_BUS                       "BUS_JSON"
   #define JS_ARG_SYS                       "SYS_JSON"
   #define JS_ARG_ENV                       "ENV_JSON"
   #define JS_ARG_OTHER                     "OTHER_JSON"

   /*
      oma create role
   */
   #define ROLE_COORD                       "coord"
   #define ROLE_CATA                        "catalog"
   #define ROLE_DATA                        "data"
   #define ROLE_STANDALONE                  "standalone"

   /*
      oma misc
   */
   #define OMA_BUFF_SIZE                    (1024)
   #define JS_FILE_NAME_LEN                 (512)
   #define JS_ARG_LEN                       (4096)
   #define WAITING_TIME                     (3000)


}

#endif // OMAGENT_DEF_HPP__

