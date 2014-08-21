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

   Source File Name = omDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/15/2014  XJH Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OM_DEF_HPP__
#define OM_DEF_HPP__

#include "core.hpp"
#include "oss.hpp"
#include "omagentDef.hpp"

namespace engine
{

   /*
      param define
   */
   #define OM_FIX_BUFF_CATCH_NUMBER          ( 100 )

   #define OM_FIX_PTR_SIZE(x)                ( x + sizeof(INT32) )
   #define OM_FIX_PTR_HEADER(ptr)            (*(INT32*)(ptr))
   #define OM_FIX_BUFF_TO_PTR(buff)          ((CHAR*)(buff)-sizeof(INT32))
   #define OM_FIX_PTR_TO_BUFF(ptr)           ((CHAR*)(ptr)+sizeof(INT32))
   #define OM_FIX_BUFF_HEADER(buff)          (*(INT32*)((CHAR*)(buff)-sizeof(INT32)))

   // max rest body size
   #define OM_REST_MAX_BODY_SIZE             ( 64 * 1024 * 1024 )

   // Session Time out
   #define OM_REST_SESSION_TIMEOUT           ( 10 * 60 * 1000 )

   #define OM_INT32_LENGTH                   (20)

   // OM user's table
   #define  OM_DEFAULT_LOGIN_USER            "admin"
   #define  OM_DEFAULT_LOGIN_PASSWD          "admin"

   /*
      OM Field Define
   */
   #define OM_CS_DEPLOY                      "SYSDEPLOY"

   // deploy.cluster
   #define OM_CS_DEPLOY_CL_CLUSTER           OM_CS_DEPLOY".SYSCLUSTER"

   #define OM_CLUSTER_FIELD_NAME             "Name"
   #define OM_CLUSTER_FIELD_DESC             "Desc"
   #define OM_CLUSTER_FIELD_SDBUSER          "SdbUser"
   #define OM_CLUSTER_FIELD_SDBPASSWD        "SdbPasswd"
   #define OM_CLUSTER_FIELD_SDBUSERGROUP     "SdbUserGroup"

   #define OM_CS_DEPLOY_CL_CLUSTERIDX1       "{name:\"SYSDEPLOY_CLUSTER_IDX1\",key: {"\
                                             OM_CLUSTER_FIELD_NAME":1}, unique: true, enforced: true } "

   // deploy.host                            
   #define OM_CS_DEPLOY_CL_HOST              OM_CS_DEPLOY".SYSHOST"

   #define OM_HOST_FIELD_NAME                "HostName"
   #define OM_HOST_FIELD_CLUSTERNAME         "ClusterName"
   #define OM_HOST_FIELD_IP                  "IP"
   #define OM_HOST_FIELD_USER                "User"
   #define OM_HOST_FIELD_PASSWORD            "Password"
   #define OM_HOST_FIELD_TIME                "Time"
   #define OM_HOST_FIELD_OS                  "OS"
   #define OM_HOST_FIELD_OM                  "OM"
   #define OM_HOST_FIELD_MEMORY              "Memory"
   #define OM_HOST_FIELD_DISK                "Disk"
   #define OM_HOST_FIELD_CPU                 "CPU"
   #define OM_HOST_FIELD_NET                 "Net"
   #define OM_HOST_FIELD_PORT                "Port"
   #define OM_HOST_FIELD_SERVICE             "Service"
   #define OM_HOST_FIELD_SAFETY              "Safety"
   #define OM_HOST_FIELD_INSTALLPATH         "InstallPath"
   #define OM_HOST_FIELD_AGENT_PORT          "AgentPort"

   #define OM_CS_DEPLOY_CL_HOSTIDX1          "{name:\"SYSDEPLOY_HOST_IDX1\",key: {"\
                                             OM_HOST_FIELD_NAME":1}, unique: true, enforced: true } "

   #define OM_CS_DEPLOY_CL_HOSTIDX2          "{name:\"SYSDEPLOY_HOST_IDX2\",key: {"\
                                             OM_HOST_FIELD_IP":1}, unique: true, enforced: true } "


   // deploy.business
   #define OM_CS_DEPLOY_CL_BUSINESS          OM_CS_DEPLOY".SYSBUSINESS"

   #define OM_BUSINESS_FIELD_NAME            "Name"
   #define OM_BUSINESS_FIELD_TYPE            "Type"
   #define OM_BUSINESS_FIELD_DEPLOYMOD       "DeployMod"
   #define OM_BUSINESS_FIELD_CLUSTERNAME     "ClusterName"
   #define OM_BUSINESS_FIELD_TIME            "Time"
   #define OM_BUSINESS_FIELD_LOCATION        "Location"

   #define OM_CS_DEPLOY_CL_BUSINESSIDX1      "{name:\"SYSDEPLOY_BUSINESS_IDX1\",key: {"\
                                             OM_BUSINESS_FIELD_NAME":1}, unique: true, enforced: true } "

   // deploy.configure
   #define OM_CS_DEPLOY_CL_CONFIGURE         OM_CS_DEPLOY".SYSCONFIGURE"

   #define OM_CONFIGURE_FIELD_HOSTNAME       "HostName"
   #define OM_CONFIGURE_FIELD_BUSINESSNAME   "BusinessName"
   #define OM_CONFIGURE_FIELD_CONFIG         "Config"

   // deploy.taskinfo
   #define OM_CS_DEPLOY_CL_TASKINFO          OM_CS_DEPLOY".SYSTASKINFO"

   #define OM_TASKINFO_FIELD_TASKID          "TaskID"
   #define OM_TASKINFO_FIELD_TYPE            "Type"
   #define OM_TASKINFO_FIELD_AGENTHOST       "AgentHost"
   #define OM_TASKINFO_FIELD_AGENTSERVICE    "AgentService"
   #define OM_TASKINFO_FIELD_INFO            "Info"
   #define OM_TASKINFO_FIELD_STATUS          "Status"

   #define OM_CS_DEPLOY_CL_TASKINFOIDX1      "{name:\"SYSDEPLOY_TASKINFO_IDX1\",key: {"\
                                             OM_TASKINFO_FIELD_TASKID":1}, unique: true, enforced: true } "

   #define OM_TASKINFO_FAKE_TASKID           "fakeID"

   // OM_REST_DEFINE
   #define  OM_REST_LOGIN_HTML               "login.html"
   #define  OM_REST_INDEX_HTML               "index.html"
   #define  OM_REST_FAVICON_ICO              "favicon.ico"

   #define  OM_REST_REDIRECT_LOGIN           "<!DOCTYPE html><html><head>"\
                                             "<meta http-equiv=\"refresh\" content=\"0;url="\
                                             OM_REST_LOGIN_HTML"\"></head></html>"

   #define  OM_REST_REDIRECT_INDEX           "<!DOCTYPE html><html><head>"\
                                             "<meta http-equiv=\"refresh\" content=\"0;url="\
                                             OM_REST_INDEX_HTML"\"></head></html>"

   #define  OM_REST_RES_RETCODE              "Rc"
   #define  OM_REST_RES_DETAIL               "Detail"
   #define  OM_REST_RES_LOCAL                "local"

   // *********************om command list****************************
   #define  OM_REST_FIELD_COMMAND            "cmd"

   // ***************om rest create_cluster_req***********************
   #define  OM_CREATE_CLUSTER_REQ            "create cluster"

   #define  OM_REST_CLUSTER_INFO             "ClusterInfo"

   #define  OM_BSON_FIELD_CLUSTER_NAME       "ClusterName"
   #define  OM_BSON_FIELD_CLUSTER_DESC       "Desc"
   #define  OM_BSON_FIELD_SDB_USER           "SdbUser"
   #define  OM_BSON_FIELD_SDB_PASSWD         "SdbPasswd"
   #define  OM_BSON_FIELD_SDB_USERGROUP      "SdbUserGroup"

   #define  OM_DEFAULT_SDB_USER              "sdbadmin"
   #define  OM_DEFAULT_SDB_PASSWD            "sdbadmin"
   #define  OM_DEFAULT_SDB_USERGROUP         "sdbadmin_group"

   // *****************************************************************

   // *********************om rest query_cluster_req*******************
   #define  OM_QUERY_CLUSTER_REQ             "query cluster"
   // *****************************************************************

   // *********************om rest login_req***************************
   #define  OM_LOGIN_REQ                     "login"

   #define  OM_REST_FIELD_LOGIN_NAME         "User"
   #define  OM_REST_FIELD_LOGIN_PASSWD       "Passwd"
   #define  OM_REST_FIELD_TIMESTAMP          "Timestamp"
   // *****************************************************************

   // *********************om rest logout_req***************************
   #define  OM_LOGOUT_REQ                    "logout"

   // *****************************************************************


   // *********************om rest change passwd***********************
   #define  OM_CHANGE_PASSWD_REQ             "change passwd"

   #define  OM_REST_FIELD_NEW_PASSWD         "NewPasswd"
   // *****************************************************************

   // *******************om rest check_session_req*********************
   #define  OM_CHECK_SESSION_REQ             "check session"
   // *****************************************************************

   // *********************om rest scan_host_req***********************
   #define  OM_SCAN_HOST_REQ                 "scan host"

   #define  OM_REST_FIELD_HOST_INFO          "HostInfo"

   #define  OM_BSON_FIELD_HOST_INFO          "HostInfo"
   #define  OM_BSON_FIELD_HOST_IP            "IP"
   #define  OM_BSON_FIELD_HOST_NAME          "HostName"
   #define  OM_BSON_FIELD_HOST_USER          "User"
   #define  OM_BSON_FIELD_HOST_PASSWD        "Passwd"
   #define  OM_BSON_FIELD_HOST_SSHPORT       "SshPort"

   #define  OM_BSON_FIELD_AGENT_PORT         "AgentPort"

   // milliseconds
   #define  OM_WAIT_SCAN_RES_INTERVAL        (60000)
   #define  OM_WAIT_PROGRESS_RES_INTERVAL    (500)

   #define  OM_WAIT_AGENT_EXIT_RES_INTERVAL  (5000)
   #define  OM_WAIT_AGENT_UNISTALL_INTERVAL  (5000)
   // *****************************************************************

   // *********************om rest check_host_req**********************
   #define  OM_CHECK_HOST_REQ                "check host"

   // sub command with agent
   #define  OM_BASIC_CHECK_REQ               "basic check"
   #define  OM_INSTALL_REMOTE_AGENT          "install remote agent"
   #define  OM_AGENT_EXIT_REQ                "exit agent"
   #define  OM_UNINSTALL_REMOTE_AGENT        "uninstall remote agent"

   #define  OM_BSON_FIELD_OS                 "OS"
   #define  OM_BSON_FIELD_OM                 "OM"
   // array
   #define  OM_BSON_FIELD_MEMORY             "Memory"
   // array
   #define  OM_BSON_FIELD_DISK               "Disk"
   #define  OM_BSON_FIELD_DISK_NAME          "Name"
   #define  OM_BSON_FIELD_DISK_SIZE          "Size"
   #define  OM_BSON_FIELD_DISK_MOUNT         "Mount"
   #define  OM_BSON_FIELD_DISK_FREE_SIZE     "Free"
   #define  OM_BSON_FIELD_DISK_USED          "Used"
   // array
   #define  OM_BSON_FIELD_CPU                "CPU"
   // array
   #define  OM_BSON_FIELD_NET                "Net"
   #define  OM_BSON_FIELD_PORT               "Port"
   #define  OM_BSON_FIELD_SERVICE            "Service"
   #define  OM_BSON_FIELD_SAFETY             "Safety"
   //
   #define  OM_BSON_FIELD_CONFIG             OM_CONFIGURE_FIELD_CONFIG

   // *****************************************************************

   // ***************om rest add_host_req******************************
   #define  OM_ADD_HOST_REQ                  "add host"

   #define  OM_ROLLBACK_TRANSACTION_REQ      "rollback transaction"

   #define  OM_BSON_FIELD_INSTALL_PATH       "InstallPath"
   #define  OM_BSON_FIELD_TRANSACTION_ID     "TransactionID"

   // with filename
   #define  OM_BSON_FIELD_PATCKET_PATH       "InstallPacket"

   #define  OM_PACKET_SUBPATH                "packet"

   // *****************************************************************

   // om rest query_host_req
   #define  OM_QUERY_HOST_REQ                "query host"
   // *****************************************************************

   // om rest query_business_type_req
   #define  OM_QUERY_BUSINESS_TYPE_REQ       "query business type"

   #define  OM_BUSINESS_CONFIG_SUBDIR        "config"
   #define  OM_BUSINESS_FILE_NAME            "business.xml"

   #define  OM_XMLATTR_KEY                   "<xmlattr>"
   #define  OM_XMLATTR_TYPE                  "<xmlattr>.type"
   #define  OM_XMLATTR_TYPE_ARRAY            "array"

   #define  OM_BSON_BUSINESS_LIST            "BusinessList"
   #define  OM_BSON_BUSINESS_TYPE            "BusinessType"
   #define  OM_BSON_BUSINESS_NAME            "BusinessName"
   #define  OM_BSON_BUSINESS_DESC            "BusinessDesc"
   // *****************************************************************

   // om rest query_business_template_req
   #define  OM_QUERY_BUSINESS_TEMPLATE_REQ   "query business template"

   #define  OM_TEMPLATE_FILE_NAME            "_template.xml"

   #define  OM_REST_BUSINESS_TYPE            OM_BSON_BUSINESS_TYPE

   #define  OM_BSON_DEPLOY_MOD_LIST          "DeployModList"

   #define  OM_BSON_DEPLOY_MOD               "DeployMod"
   #define  OM_BSON_PROPERTY_ARRAY           "Property"
   #define  OM_BSON_PROPERTY_NAME            "Name"
   #define  OM_BSON_PROPERTY_TYPE            "Type"
   #define  OM_BSON_PROPERTY_DEFAULT         "Default"
   #define  OM_BSON_PROPERTY_VALID           "Valid"
   #define  OM_BSON_PROPERTY_DISPLAY         "Display"
   #define  OM_BSON_PROPERTY_EDIT            "Edit"
   #define  OM_BSON_PROPERTY_DESC            "Desc"
   #define  OM_BSON_PROPERTY_LEVEL           "Level"
   #define  OM_BSON_PROPERTY_WEBNAME         "WebName"

   // *****************************************************************

   // om rest config_business_req
   #define  OM_CONFIG_BUSINESS_REQ           "config business"

   #define  OM_CONFIG_ITEM_FILE_NAME         "_config.xml"
   #define  OM_XML_CONFIG                    "config"

   #define  OM_REST_TEMPLATE_INFO            "TemplateInfo"

   #define  OM_BSON_PROPERTY_VALUE           "Value"

   // *****************************************************************

   // om rest install_business_req
   #define  OM_INSTALL_BUSINESS_REQ          "add business"

   #define  OM_ROLLBACK_INSTALL_REQ          OM_ROLLBACK_TRANSACTION_REQ

   #define  OM_REST_CONFIG_INFO              "ConfigInfo"

   #define  OM_BSON_TASKID                   "TaskID"
   // *****************************************************************

   // om query install progress
   #define  OM_QUERY_PROGRESS                "query progress"

   #define  OM_REST_TASK_INFO                "Task"

   #define  OM_BSON_ISFINISHED               "IsFinish"
   #define  OM_BSON_TASK_STATUS              "Status"
   #define  OM_BSON_TASK_PROGRESS            "Progress"
   #define  OM_BSON_ITEM_NAME                "Name"
   #define  OM_BSON_TOTAL_COUNT              "TotalCount"
   #define  OM_BSON_INSTALLED_COUNT          "InstalledCount"
   #define  OM_BSON_ITEM_DESC                "Desc"

   #define  OM_TASK_STATUS_INSTALL           "install"
   #define  OM_TASK_STATUS_ROLLBACK          "rollback"
   //******************************************************************

   // om query node req
   #define  OM_QUERY_NODE_REQ                "query node"

   #define  OM_REST_BUSINESS_NAME            OM_BSON_BUSINESS_NAME

   #define  OM_BSON_NODE_INFO                "NodeInfo"
   
   //******************************************************************

   // om query business req
   #define  OM_QUERY_BUSINESS_REQ            "query business"

   #define  OM_REST_CLUSTER_NAME             "ClusterName"
   #define  OM_BSON_BUSINESS_INFO            "BusinessInfo"
   //******************************************************************

   // om query business req
   //******************************************************************

   // om start business req
   //******************************************************************

   // om stop business req
   //******************************************************************

   // milliseconds
   #define  OM_WAIT_EVENT_INTERVAL           (500)



   // agent default localhost
   #define  OM_DEFAULT_LOCAL_HOST            "localhost"
   // agent default port
   #define  OM_AGENT_DEFAULT_PORT            SDBCM_DFT_PORT


}

#endif // OM_DEF_HPP__

