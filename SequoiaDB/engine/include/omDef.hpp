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

   #define OM_CS_DEPLOY_CL_CLUSTERIDX1       "{name:\"SYSDEPLOY_CLUSTER_IDX1\",key: {"\
                                             OM_CLUSTER_FIELD_NAME":1}, unique: true, enforced: true } "

   // deploy.host                            
   #define OM_CS_DEPLOY_CL_HOST              OM_CS_DEPLOY".SYSHOST"
   
   #define OM_HOST_FIELD_NAME                "Name"
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

   #define OM_CS_DEPLOY_CL_HOSTIDX1          "{name:\"SYSDEPLOY_HOST_IDX1\",key: {"\
                                             OM_HOST_FIELD_NAME":1}, unique: true, enforced: true } "

   #define OM_CS_DEPLOY_CL_HOSTIDX2          "{name:\"SYSDEPLOY_HOST_IDX2\",key: {"\
                                             OM_HOST_FIELD_IP":1}, unique: true, enforced: true } "
   

   // deploy.business
   #define OM_CS_DEPLOY_CL_BUSINESS          OM_CS_DEPLOY".SYSBUSINESS"
    
   #define OM_BUSINESS_FIELD_NAME            "Name"
   #define OM_BUSINESS_FIELD_TYPE            "Type"
   #define OM_BUSINESS_FIELD_CLUSTERNAME     "ClusterName"
   #define OM_BUSINESS_FIELD_TIME            "Time"
   #define OM_BUSINESS_FIELD_LOCATION        "Location"

   #define OM_CS_DEPLOY_CL_BUSINESSIDX1      "{name:\"SYSDEPLOY_BUSINESS_IDX1\",key: {"\
                                             OM_BUSINESS_FIELD_NAME":1}, unique: true, enforced: true } "

   // deploy.configure
   #define OM_CS_DEPLOY_CL_CONFIGURE         OM_CS_DEPLOY".SYSCONFIGURE"

   #define OM_CONFIGURE_FIELD_HOSTNAME       "HostName"
   #define OM_CONFIGURE_FIELD_SERVICENAME    "ServiceName"
   #define OM_CONFIGURE_FIELD_BUSINESSNAME   "BusinessName"
   #define OM_CONFIGURE_FIELD_CONFIG         "Config"

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
   
   #define  OM_REST_RES_RETCODE              "rc"
   #define  OM_REST_RES_DETAIL               "detail"
   #define  OM_REST_RES_LOCAL                "local"

   // ****************om command list********************************
   #define  OM_REST_FIELD_COMMAND            "cmd"

   // om rest create_cluster_req
   #define  OM_CREATE_CLUSTER_REQ            "create_cluster_req"
   
   #define  OM_REST_FIELD_CLUSTER_INFO       "cluster_info"

   #define  OM_BSON_FIELD_CLUSTER_NAME       "cluster_name"
   #define  OM_BSON_FIELD_CLUSTER_DESC       "desc"

   // om rest query_cluster_req
   #define  OM_QUERY_CLUSTER_REQ             "query_cluster_req"

   // om rest login_req
   #define  OM_LOGIN_REQ                     "login_req"
   
   #define  OM_REST_FIELD_LOGIN_NAME         "user"
   #define  OM_REST_FIELD_LOGIN_PASSWD       "passwd"
   #define  OM_REST_FIELD_TIMESTAMP          "timestamp"

   // om rest check_session_req
   #define  OM_CHECK_SESSION_REQ             "check_session_req"

   // om rest scan_host_req
   #define  OM_SCAN_HOST_REQ                 "scan_host_req"

   #define  OM_REST_FIELD_HOST_INFO          "host_info"
   
   #define  OM_BSON_FIELD_HOST_IP            "host_ip"
   #define  OM_BSON_FIELD_HOST_NAME          "host_name"
   #define  OM_BSON_FIELD_HOST_USER          "username"
   #define  OM_BSON_FIELD_HOST_PASSWD        "passwd"
   #define  OM_BSON_FIELD_HOST_SSHPORT       "port"

   // om rest check_host_req
   #define  OM_CHECK_HOST_REQ                "check_host_req"
   //****************sub command with agent***************************
   #define  OM_BASIC_CHECK_REQ               "basic_check_req"
   #define  OM_INSTALL_REMOTE_AGENT          "install_remote_agent_req"
   //******************************************************************


   // om rest add_host_req
   #define  OM_ADD_HOST_REQ                  "add_host_req"
   




   // milliseconds
   #define  OM_WAIT_EVENT_INTERVAL           (500)
   
}

#endif // OM_DEF_HPP__

