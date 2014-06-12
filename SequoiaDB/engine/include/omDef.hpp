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

   /*
      HTML File Name Define
   */
   #define HTML_FILE_LOGIN                   "login.html"
   #define HTML_FILE_INDEX                   "index.html"

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
   
}

#endif // OM_DEF_HPP__

