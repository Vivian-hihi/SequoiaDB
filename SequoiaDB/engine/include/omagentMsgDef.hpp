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

   Source File Name = omagentMsgDef.hpp

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          06/30/2014  TZB Initial Draft

   Last Changed =

*******************************************************************************/

#ifndef OMAGENT_MSG_DEF_HPP__
#define OMAGENT_MSG_DEF_HPP__

#include "pmdOptions.hpp"

#define OMA_FIELD_HOSTS                            "Hosts"
#define OMA_FIELD_HOSTNAME                         "Hostname"
#define OMA_FIELD_HOSTNAME1                        "HostName"
#define OMA_FIELD_USER                             "User"
#define OMA_FIELD_PASSWORD                         "Password"
#define OMA_FIELD_IP                               "IP"

#define OMA_FIELD_PING                             "Ping"
#define OMA_FIELD_SSH                              "Ssh"
#define OMA_FIELD_LOCAL_PACKET_PATH                "LocalPacketPath"
#define OMA_FIELD_REMOTE_PACKET_PATH               "RemotePacketPath"
#define OMA_FIELD_AGENT_IS_RUNNING                 "IsRunning"
#define OMA_FIELD_INSTALL_PATH                     "InstallPath"

#define OMA_FIELD_RC                               "Rc"
#define OMA_FIELD_DETAIL                           "Detail"
#define OMA_FIELD_HASRUNNING                       "HasRunning"
#define OMA_FIELD_HASPUSH                          "HasPush"
#define OMA_FIELD_PORTHASUSED                      "PortHasUsed"
#define OMA_FIELD_TASKID                           "TaskID"

// host info
#define OMA_FIELD_HOSTSTABLE                       "HostsTable"
#define OMA_FIELD_OS                               "OS"
#define OMA_FIELD_OM                               "OM"
#define OMA_FIELD_TIME                             "Time"
#define OMA_FIELD_MEMORY                           "Memory"
#define OMA_FIELD_DISK                             "Disk"
#define OMA_FIELD_CPU                              "CPU"
#define OMA_FIELD_Net                              "Net"
#define OMA_FIELD_PORT                             "Port"
#define OMA_FIELD_SERVICE                          "Service"
#define OMA_FIELD_SAFETY                           "Safety"

// om
#define OMA_FIELD_VERSION                          "Version"
#define OMA_FIELD_Path                             "Path"

// memory
#define OMA_FIELD_SIZE                             "Size"
#define OMA_FIELD_MODEL                            "Model"
#define OMA_FIELD_FREE                             "Free"

// Disk
#define OMA_FIELD_NAME                             "Name"
#define OMA_FIELD_Mount                            "Mount"
#define OMA_FIELD_USED                             "Used"

// cpu
#define OMA_FIELD_ID                               "ID"
#define OMA_FIELD_CORE                             "Core"
#define OMA_FIELD_FREQ                             "Freq"

// net
#define OMA_FIELD_BANDWIDTH                        "Bandwidth"

// port
#define OMA_FIELD_STATUS                           "Status"

// service

// safety
#define OMA_FIELD_CONTEXT                          "Context"

// business
#define OMA_FIELD_CLUSTERNAME                      "ClusterName"

// config file
#define OMA_OPTION_DATAGROUPNAME                   "datagroupname"
#define OMA_OPTION_HELP                            PMD_OPTION_HELP
#define OMA_OPTION_VERSION                         PMD_OPTION_VERSION
#define OMA_OPTION_DBPATH                          PMD_OPTION_DBPATH
#define OMA_OPTION_IDXPATH                         PMD_OPTION_IDXPATH
#define OMA_OPTION_CONFPATH                        PMD_OPTION_CONFPATH
#define OMA_OPTION_LOGPATH                         PMD_OPTION_LOGPATH
#define OMA_OPTION_DIAGLOGPATH                     PMD_OPTION_DIAGLOGPATH
#define OMA_OPTION_DIAGLOG_NUM                     PMD_OPTION_DIAGLOG_NUM
#define OMA_OPTION_BKUPPATH                        PMD_OPTION_BKUPPATH
#define OMA_OPTION_MAXPOOL                         PMD_OPTION_MAXPOOL
#define OMA_OPTION_SVCNAME                         PMD_OPTION_SVCNAME
#define OMA_OPTION_REPLNAME                        PMD_OPTION_REPLNAME
#define OMA_OPTION_SHARDNAME                       PMD_OPTION_SHARDNAME
#define OMA_OPTION_CATANAME                        PMD_OPTION_CATANAME
#define OMA_OPTION_RESTNAME                        PMD_OPTION_RESTNAME
#define OMA_OPTION_DIAGLEVEL                       PMD_OPTION_DIAGLEVEL
#define OMA_OPTION_ROLE                            PMD_OPTION_ROLE
#define OMA_OPTION_CATALOG_ADDR                    PMD_OPTION_CATALOG_ADDR
#define OMA_OPTION_LOGFILESZ                       PMD_OPTION_LOGFILESZ
#define OMA_OPTION_LOGFILENUM                      PMD_OPTION_LOGFILENUM
#define OMA_OPTION_TRANSACTIONON                   PMD_OPTION_TRANSACTIONON
#define OMA_OPTION_NUMPRELOAD                      PMD_OPTION_NUMPRELOAD
#define OMA_OPTION_MAX_PREF_POOL                   PMD_OPTION_MAX_PREF_POOL
#define OMA_OPTION_MAX_SUB_QUERY                   PMD_OPTION_MAX_SUB_QUERY
#define OMA_OPTION_MAX_REPL_SYNC                   PMD_OPTION_MAX_REPL_SYNC
#define OMA_OPTION_LOGBUFFSIZE                     PMD_OPTION_LOGBUFFSIZE
#define OMA_OPTION_DMS_TMPBLKPATH                  PMD_OPTION_DMS_TMPBLKPATH
#define OMA_OPTION_SORTBUF_SIZE                    PMD_OPTION_SORTBUF_SIZE
#define OMA_OPTION_HJ_BUFSZ                        PMD_OPTION_HJ_BUFSZ
#define OMA_OPTION__SYNC_STRATEGY                  PMD_OPTION_SYNC_STRATEGY
#define OMA_OPTION_REPL_BUCKET_SIZE                PMD_OPTION_REPL_BUCKET_SIZE
#define OMA_OPTION_MEMDEBUG                        PMD_OPTION_MEMDEBUG
#define OMA_OPTION_MEMDEBUGSIZE                    PMD_OPTION_MEMDEBUGSIZE
#define OMA_OPTION_CATALIST                        PMD_OPTION_CATALIST
#define OMA_OPTION_DPSLOCAL                        PMD_OPTION_DPSLOCAL
#define OMA_OPTION_TRACEON                         PMD_OPTION_TRACEON
#define OMA_OPTION_TRACEBUFSZ                      PMD_OPTION_TRACEBUFSZ
#define OMA_OPTION_SHARINGBRK                      PMD_OPTION_SHARINGBRK
#define OMA_OPTION_INDEX_SCAN_STEP                 PMD_OPTION_INDEX_SCAN_STEP
#define OMA_OPTION_START_SHIFT_TIME                PMD_OPTION_START_SHIFT_TIME
#define OMA_OPTION_PREFINST                        PMD_OPTION_PREFINST
#define OMA_OPTION_NUMPAGECLEANERS                 PMD_OPTION_NUMPAGECLEANERS
#define OMA_OPTION_PAGECLEANINTERVAL               PMD_OPTION_PAGECLEANINTERVAL


#define OMA_FIELD_SCAN_HOST_RET                    "ScanHostRet"
#define OMA_FIELD_INSATLL_REMOTE_AGENT_RET         "InstallRemoteAgentRet"
#define OMA_FIELD_CHECK_REMOTE_AGENT_PROCESS_RET   "CheckRemoteAgentProcessRet"
#define OMA_FIELD_INSTALL_AGENT_PROCESS_RET        "InstallAgentProcessRet"
#define OMA_FIELD_GET_HOST_INFO_RET                "GetHostInfoRet"
#define OMA_FIELD_GET_HOST_NAME_RET                "GetHostNameRet"


#endif
