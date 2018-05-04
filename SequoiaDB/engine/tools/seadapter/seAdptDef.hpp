/*******************************************************************************


   Copyright (C) 2011-2018 SequoiaDB Ltd.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   Source File Name = seAdptDef.hpp

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbcm,
   which is used to do cluster managing.

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          04/14/2017  YSD  Initial Draft

   Last Changed =

*******************************************************************************/
#ifndef SEADPT_DEF_HPP_
#define SEADPT_DEF_HPP_

namespace seadapter
{
   #define SDB_SEADPT_PROCESS_NAME     "sdbseadapter"
   #define SDB_SEADPT_ROLE_SHORT_STR   "A"
   #define SDB_SEADPT_DNODE_HOST       "datanodehost"
   #define SDB_SEADPT_DNODE_PORT       "datasvcname"
   #define SDB_SEADPT_DIAGLEVEL        "diaglevel"
   #define SDB_SEADPT_SE_HOST          "searchenginehost"
   #define SDB_SEADPT_SE_PORT          "searchengineport"
   #define SDB_SEADPT_BULK_BUFF_SIZE   "bulkbuffsize"
   #define SDB_SEADPT_GRP_ID           65536
   #define SDB_SEADPT_NODE_ID          0
   #define SDB_SEADPT_SVC_ID           0
   #define SDB_SEADPT_MAX_IDXNAME_SZ   255
   #define SDB_SEADPT_COMMIT_ID        "SDBCOMMIT"

   #define SDB_SEADPT_EXE_FILE_NAME    "sdbseadapter"
   #define SDB_SEADPT_CFG_FILE_NAME    SDB_SEADPT_EXE_FILE_NAME".conf"
   #define SDB_SEADPT_LOG_DIR          "sdbseadapterlog"
   #define SDB_SEADPT_LOG_FILE_NAME    SDB_SEADPT_EXE_FILE_NAME".log"
   #define SDB_SEADPT_LOCK_FILE_NAME   ".sdbseadapter.lock"
}

#endif /* SEADPT_DEF_HPP_ */

