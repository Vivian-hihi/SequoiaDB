/** \file ossErr.h
    \brief The meaning of the error code.
*/
/*    Copyright 2012 SequoiaDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// This Header File is automatically generated, you MUST NOT modify this file anyway!
// On the contrary, you can modify the xml file "sequoiadb/misc/autogen/rclist.xml" if necessary!

#ifndef OSSERR_H_
#define OSSERR_H_

#include "core.h"
#include "ossFeat.h"

#define SDB_MAX_ERROR                   1024
#define SDB_MAX_WARNING                 1024
#define SDB_OK                          0

/** \fn CHAR* getErrDesp ( INT32 errCode )
    \brief Error Code.
    \param [in] errCode The number of the error code
    \returns The meaning of the error code
 */
const CHAR* getErrDesp ( INT32 errCode );

#define SDB_IO                          -1    /**< IO Exception */
#define SDB_OOM                         -2    /**< Out of Memory */
#define SDB_PERM                        -3    /**< Permission Error */
#define SDB_FNE                         -4    /**< File Not Exist */
#define SDB_FE                          -5    /**< File Exist */
#define SDB_INVALIDARG                  -6    /**< Invalid Argument */
#define SDB_INVALIDSIZE                 -7    /**< Invalid size */
#define SDB_INTERRUPT                   -8    /**< Interrupt */
#define SDB_EOF                         -9    /**< hit end of file */
#define SDB_SYS                         -10   /**< system error */
#define SDB_NOSPC                       -11   /**< has no space */
#define SDB_EDU_INVAL_STATUS            -12   /**< EDU status is not valid */
#define SDB_TIMEOUT                     -13   /**< Timeout error */
#define SDB_QUIESCED                    -14   /**< Database is quiesced */
#define SDB_NETWORK                     -15   /**< Network error */
#define SDB_NETWORK_CLOSE               -16   /**< Network is closed from remote */
#define SDB_DATABASE_DOWN               -17   /**< Database is in shutdown status */
#define SDB_APP_FORCED                  -18   /**< Application is forced */
#define SDB_INVALIDPATH                 -19   /**< Given path is not valid */
#define SDB_INVALID_FILE_TYPE           -20   /**< Unexpected file type specified */
#define SDB_DMS_NOSPC                   -21   /**< There's no space for DMS */
#define SDB_DMS_EXIST                   -22   /**< collection already exist */
#define SDB_DMS_NOTEXIST                -23   /**< collection does not exist */
#define SDB_DMS_RECORD_TOO_BIG          -24   /**< user record is too big */
#define SDB_DMS_RECORD_NOTEXIST         -25   /**< record does not exist */
#define SDB_DMS_OVF_EXIST               -26   /**< remote overflow record exist */
#define SDB_DMS_RECORD_INVALID          -27   /**< invalid record */
#define SDB_DMS_SU_NEED_REORG           -28   /**< storage unit need reorg */
#define SDB_DMS_EOC                     -29   /**< end of collection */
#define SDB_DMS_CONTEXT_IS_OPEN         -30   /**< context is already opened */
#define SDB_DMS_CONTEXT_IS_CLOSE        -31   /**< context is closed */
#define SDB_OPTION_NOT_SUPPORT          -32   /**< option is not supported yet */
#define SDB_DMS_CS_EXIST                -33   /**< collection space already exist */
#define SDB_DMS_CS_NOTEXIST             -34   /**< collection space not exist */
#define SDB_DMS_INVALID_SU              -35   /**< storage unit file is invalid */
#define SDB_RTN_CONTEXT_NOTEXIST        -36   /**< context not exist */
#define SDB_IXM_MULTIPLE_ARRAY          -37   /**< more than one field has array */
#define SDB_IXM_DUP_KEY                 -38   /**< duplicate key exist */
#define SDB_IXM_KEY_TOO_LARGE           -39   /**< key is too large */
#define SDB_IXM_NOSPC                   -40   /**< index extent has no space */
#define SDB_IXM_KEY_NOTEXIST            -41   /**< index key not exist */
#define SDB_DMS_MAX_INDEX               -42   /**< hit max number of index */
#define SDB_DMS_INIT_INDEX              -43   /**< failed to initialize index */
#define SDB_DMS_COL_DROPPED             -44   /**< collection is dropped */
#define SDB_IXM_IDENTICAL_KEY           -45   /**< two records get same key and rid */
#define SDB_IXM_EXIST                   -46   /**< duplicate index name */
#define SDB_IXM_NOTEXIST                -47   /**< index name doesn't exist */
#define SDB_IXM_UNEXPECTED_STATUS       -48   /**< index flag is unexpected */
#define SDB_IXM_EOC                     -49   /**< hit end of index */
#define SDB_IXM_DEDUP_BUF_MAX           -50   /**< hit max of dedup buffer */
#define SDB_RTN_INVALID_PREDICATES      -51   /**< invalid predicates */
#define SDB_RTN_INDEX_NOTEXIST          -52   /**< index is no longer exist */
#define SDB_RTN_INVALID_HINT            -53   /**< index hint is not valid */
#define SDB_DMS_NO_MORE_TEMP            -54   /**< no more temp tables avaliable */
#define SDB_DMS_SU_OUTRANGE             -55   /**< exceed max number of SU */
#define SDB_IXM_DROP_ID                 -56   /**< $id index can't be dropped */
#define SDB_DPS_LOG_NOT_IN_BUF          -57   /**< log was not found in log buf */
#define SDB_DPS_LOG_NOT_IN_FILE         -58   /**< log was not found in log file */
#define SDB_PMD_RG_NOT_EXIST            -59   /**< replication group not exist */
#define SDB_PMD_RG_EXIST                -60   /**< replication group exist */
#define SDB_INVALID_REQID               -61   /**< invalid request id is received */
#define SDB_PMD_SESSION_NOT_EXIST       -62   /**< session ID does not exist */
#define SDB_PMD_FORCE_SYSTEM_EDU        -63   /**< system edu can't be forced */
#define SDB_NOT_CONNECTED               -64   /**< database is not connected */
#define SDB_UNEXPECTED_RESULT           -65   /**< unexpected result received */
#define SDB_CORRUPTED_RECORD            -66   /**< corrupted record */
#define SDB_BACKUP_HAS_ALREADY_START    -67   /**< backup has already started */
#define SDB_BACKUP_NOT_COMPLETE         -68   /**< backup not completed */
#define SDB_RTN_IN_BACKUP               -69   /**< backup in progress mode */
#define SDB_BAR_DAMAGED_BK_FILE         -70   /**< backup file is damaged */
#define SDB_RTN_NO_PRIMARY_FOUND        -71   /**< there's no primary found */
#define SDB_CAT_NODE_NOT_FOUND          -72   /**< the requested node not exist */
#define SDB_PMD_HELP_ONLY               -73   /**< engine help argument is specified */
#define SDB_PMD_CON_INVALID_STATE       -74   /**< connection state is not valid */
#define SDB_CLT_INVALID_HANDLE          -75   /**< invalid handle */
#define SDB_CLT_OBJ_NOT_EXIST           -76   /**< client object is freed */
#define SDB_NET_ALREADY_LISTENED        -77   /**< transfer has already listened */
#define SDB_NET_CANNOT_LISTEN           -78   /**< can not listen specified addr */
#define SDB_NET_CANNOT_CONNECT          -79   /**< cannot connect to specified addr */
#define SDB_NET_NOT_CONNECT             -80   /**< connection does not exist */
#define SDB_NET_SEND_ERR                -81   /**< failed to send */
#define SDB_NET_TIMER_ID_NOT_FOUND      -82   /**< timer id not found */
#define SDB_NET_ROUTE_NOT_FOUND         -83   /**< route info not found */
#define SDB_NET_BROKEN_MSG              -84   /**< broken msg */
#define SDB_NET_INVALID_HANDLE          -85   /**< invalid net handle */
#define SDB_DMS_INVALID_REORG_FILE      -86   /**< reorg file is not valid */
#define SDB_DMS_REORG_FILE_READONLY     -87   /**< reorg file is in read only mode */
#define SDB_DMS_INVALID_COLLECTION_S    -88   /**< collection status is not valid */
#define SDB_DMS_NOT_IN_REORG            -89   /**< collection is not in reorg state */
#define SDB_REPL_GROUP_NOT_ACTIVE       -90   /**< replication group is not activated */
#define SDB_REPL_INVALID_GROUP_MEMBER   -91   /**< the member is not in this group */
#define SDB_DMS_INCOMPATIBLE_MODE       -92   /**< collection flag not compatible */
#define SDB_DMS_INCOMPATIBLE_VERSION    -93   /**< stroage unit version not compatible */
#define SDB_REPL_LOCAL_G_V_EXPIRED      -94   /**< local group version is expired */
#define SDB_DMS_INVALID_PAGESIZE        -95   /**< page size is not valid */
#define SDB_REPL_REMOTE_G_V_EXPIRED     -96   /**< remote group version is expired */
#define SDB_CLS_VOTE_FAILED             -97   /**< failed to create a poll */
#define SDB_DPS_CORRUPTED_LOG           -98   /**< log record is corrupted */
#define SDB_DPS_LSN_OUTOFRANGE          -99   /**< given lsn greater than latest */
#define SDB_UNKNOWN_MESSAGE             -100  /**< received unknown mesage */
#define SDB_NET_UPDATE_EXISTING_NODE    -101  /**< updated information is same as old one */
#define SDB_CLS_UNKNOW_MSG              -102  /**< unknow message */
#define SDB_CLS_EMPTY_HEAP              -103  /**< empty heap */
#define SDB_CLS_NOT_PRIMARY             -104  /**< not primary */
#define SDB_CLS_NODE_NOT_ENOUGH         -105  /**< data node not enough */
#define SDB_CLS_NO_CATALOG_INFO         -106  /**< data node have no catalog info */
#define SDB_CLS_DATA_NODE_CAT_VER_OLD   -107  /**< data node catlog version old */
#define SDB_CLS_COORD_NODE_CAT_VER_OLD  -108  /**< coord node catalog version old */
#define SDB_CLS_INVALID_GROUP_NUM       -109  /**< exceds the max group size */
#define SDB_CLS_SYNC_FAILED             -110  /**< failed to sync log */
#define SDB_CLS_REPLAY_LOG_FAILED       -111  /**< failed to replay log */
#define SDB_REST_EHS                    -112  /**< error http struct */
#define SDB_CLS_CONSULT_FAILED          -113  /**< failed to negotiate */
#define SDB_DPS_MOVE_FAILED             -114  /**< failed to change dps metadata */
#define SDB_DMS_CORRUPTED_SME           -115  /**< SME is corrupted */
#define SDB_APP_INTERRUPT               -116  /**< application is interrupted */
#define SDB_APP_DISCONNECT              -117  /**< application is disconnected */
#define SDB_OSS_CCE                     -118  /**< character encoding errors */
#define SDB_COORD_QUERY_FAILED          -119  /**< get failed msg from the node */
#define SDB_CLS_BUFFER_FULL             -120  /**< buffer array is full */
#define SDB_RTN_SUBCONTEXT_CONFLICT     -121  /**< sub context is conflict */
#define SDB_COORD_QUERY_EOC             -122  /**< coord received EOC message */
#define SDB_DPS_FILE_SIZE_NOT_SAME      -123  /**< DPS file size are not the same */
#define SDB_DPS_FILE_NOT_RECOGNISE      -124  /**< dps file not recognise */
#define SDB_OSS_NORES                   -125  /**< no resource */
#define SDB_DPS_INVALID_LSN             -126  /**< invalid lsn */
#define SDB_OSS_NPIPE_DATA_TOO_BIG      -127  /**< data to pipe is too big */
#define SDB_CAT_AUTH_FAILED             -128  /**< catalog auth failed */
#define SDB_CLS_FULL_SYNC               -129  /**< node is full sync */
#define SDB_CAT_ASSIGN_NODE_FAILED      -130  /**< catnode failed assign data-node */
#define SDB_PHP_DRIVER_INTERNAL_ERROR   -131  /**< php driver internal error */
#define SDB_COORD_SEND_MSG_FAILED       -132  /**< failed to send the message */
#define SDB_CAT_NO_NODEGROUP_INFO       -133  /**< there is no node-group register in catalogue-node */
#define SDB_COORD_REMOTE_DISC           -134  /**< remote-node disconnected */
#define SDB_CAT_NO_MATCH_CATALOG        -135  /**< couldn't find the match catalogue-info */
#define SDB_CLS_UPDATE_CAT_FAILED       -136  /**< update catalog failed */
#define SDB_COORD_UNKNOWN_OP_REQ        -137  /**< received invalid request which opcode is unknowned */
#define SDB_COOR_NO_NODEGROUP_INFO      -138  /**< coord couldn't find the group-info in local */
#define SDB_DMS_CORRUPTED_EXTENT        -139  /**< dms extent is corrupted */
#define SDBCM_FAIL                      -140  /**< remote cluster manage failed */
#define SDBCM_STOP_PART                 -141  /**< remote engines have been stopped partially */
#define SDBCM_SVC_STARTING              -142  /**< sdb service is starting */
#define SDBCM_SVC_STARTED               -143  /**< sdb service has already been started */
#define SDBCM_SVC_RESTARTING            -144  /**< sdb service is restarting */
#define SDBCM_NODE_EXISTED              -145  /**< sdb node is already existed */
#define SDBCM_NODE_NOTEXISTED           -146  /**< sdb node is not existed */
#define SDB_LOCK_FAILED                 -147  /**< unable to lock */
#define SDB_DMS_STATE_NOT_COMPATIBLE    -148  /**< dms state is not compatible with current command */
#define SDB_REBUILD_HAS_ALREADY_START   -149  /**< rebuild has already started */
#define SDB_RTN_IN_REBUILD              -150  /**< rebuild in progress mode */
#define SDB_RTN_COORD_CACHE_EMPTY       -151  /**< there is no data in the coord-node's cache  */
#define SDB_SPT_EVAL_FAIL               -152  /**< errors occur during the process of evalution */
#define SDB_CAT_GRP_EXIST               -153  /**< group already exist */
#define SDB_CLS_GRP_NOT_EXIST           -154  /**< group does not exist */
#define SDB_CLS_NODE_NOT_EXIST          -155  /**< node does not exist */
#define SDB_CM_RUN_NODE_FAILED          -156  /**< failed to start the node */
#define SDB_CM_CONFIG_CONFLICTS         -157  /**< node'configure conflicts */
#define SDB_CLS_EMPTY_GROUP             -158  /**< group is empty */
#define SDB_RTN_COORD_ONLY              -159  /**< The operation is for coord node only */
#define SDB_CM_OP_NODE_FAILED           -160  /**< failed to operate on node only */
#define SDB_RTN_MUTEX_JOB_EXIST         -161  /**< The mutex job already exist */
#define SDB_RTN_JOB_NOT_EXIST           -162  /**< The specified job does not exist */
#define SDB_CAT_CORRUPTION              -163  /**< The catalog information is corrupted */
#define SDB_IXM_DROP_SHARD              -164  /**< $shard index can't be dropped */
#define SDB_RTN_CMD_NO_NODE_AUTH        -165  /**< The command can't be run in the node */
#define SDB_RTN_CMD_NO_SERVICE_AUTH     -166  /**< The command can't be run in the serice plane */
#define SDB_CLS_NO_GROUP_INFO           -167  /**< The group info not exist */
#define SDB_CLS_GROUP_NAME_CONFLICT     -168  /**< Group name is conflict */
#define SDB_COLLECTION_NOTSHARD         -169  /**< The collection is not sharded */
#define SDB_INVALID_SHARDINGKEY         -170  /**< The record does not contains valid sharding key */
#define SDB_TASK_EXIST                  -171  /**< A task that already exists does not compatible with the new task */
#define SDB_CL_NOT_EXIST_ON_GROUP       -172  /**< The collection does not exists on the specified group */
#define SDB_CAT_TASK_NOTFOUND           -173  /**< The specified task does not exist */
#define SDB_MULTI_SHARDING_KEY          -174  /**< The record contains more than one sharding key */
#define SDB_CLS_MUTEX_TASK_EXIST        -175  /**< The mutex task already exist */
#define SDB_CLS_BAD_SPLIT_KEY           -176  /**< The split key is not valid or not in the source group */
#define SDB_SHARD_KEY_NOT_IN_UNIQUE_KEY -177  /**< The unique index must include all fields in sharding key */
#define SDB_UPDATE_SHARD_KEY            -178  /**< Sharding key cannot be updated */
#define SDB_AUTH_AUTHORITY_FORBIDDEN    -179  /**< authority is forbidden */
#define SDB_CAT_NO_ADDR_LIST            -180  /**< There is no catalog address specified by user */
#define SDB_CURRENT_RECORD_DELETED      -181  /**< current record has been deleted */
#define SDB_QGM_MATCH_NONE              -182  /**< search condition cannot match any records */
#define SDB_IXM_REORG_DONE              -183  /**< index page is reorged and the pos got different lchild */
#define SDB_RTN_DUPLICATE_FIELDNAME     -184  /**< There are duplicate field name exists in the record */
#define SDB_QGM_MAX_NUM_RECORD          -185  /**< Too many records to be inserted at once */
#define SDB_QGM_MERGE_JOIN_EQONLY       -186  /**< Sort-Merge Join only supports equal predicates */
#define SDB_PD_TRACE_IS_STARTED         -187  /**< Trace is already started */
#define SDB_PD_TRACE_HAS_NO_BUFFER      -188  /**< Trace buffer does not exist */
#define SDB_PD_TRACE_FILE_INVALID       -189  /**< Trace file is not valid */
#define SDB_DPS_TRANS_LOCK_INCOMPATIBLE -190  /**< transaction-lock is incompatible */
#define SDB_DPS_TRANS_DOING_ROLLBACK    -191  /**< system is doing rollback */
#define SDB_MIG_IMP_BAD_RECORD          -192  /**< Bad record when doing sdb import  */
#define SDB_QGM_REPEAT_VAR_NAME         -193  /**< repeat var name was found */
#define SDB_QGM_AMBIGUOUS_FIELD         -194  /**< column field is ambiguous */
#define SDB_SQL_SYNTAX_ERROR            -195  /**< have an error in sql syntax */
#define SDB_DPS_TRANS_NO_TRANS          -196  /**< invalid transactional operation */
#define SDB_DPS_TRANS_APPEND_TO_WAIT    -197  /**< append to lock-wait-queue */
#define SDB_DMS_DELETING                -198  /**< record is deleting */
#define SDB_DMS_INVALID_INDEXCB         -199  /**< index is dropped or invalid */
#define SDB_COORD_RECREATE_CATALOG      -200  /**< repeat to create catalog-group */
#define SDB_UTIL_PARSE_JSON_INVALID     -201  /**< parse json error */
#define SDB_UTIL_PARSE_CSV_INVALID      -202  /**< parse CSV error */
#define SDB_DPS_LOG_FILE_OUT_OF_SIZE    -203  /**< log file is out of size */
#define SDB_CATA_RM_NODE_FORBIDDEN      -204  /**< can not remove the only node in the group */
#define SDB_CATA_FAILED_TO_CLEANUP      -205  /**< need to manually complete the cleanup */
#define SDB_CATA_RM_CATA_FORBIDDEN      -206  /**< can not remove node or group of catalog */
#define SDB_CAT_GRP_NOT_EXIST           -207  /**< group is not exist */
#define SDB_CAT_RM_GRP_FORBIDDEN        -208  /**< can not remove the group with data in it */
#define SDB_MIG_END_OF_QUEUE            -209  /**< end of queue */
#define SDB_COORD_SPLIT_NO_SHDIDX       -210  /**< collection has not sharding index, can not split by percent */
#define SDB_FIELD_NOT_EXIST             -211  /**< the param field not exist */
#define SDB_TOO_MANY_TRACE_BP           -212  /**< too many trace break points are specified */
#define SDB_BUSY_PREFETCHER             -213  /**< prefetchers are all busy */
#define SDB_CAT_DOMAIN_NOT_EXIST        -214  /**< domain not exist */
#define SDB_CAT_DOMAIN_EXIST            -215  /**< domain already exist */
#define SDB_CAT_GROUP_NOT_IN_DOMAIN     -216  /**< group is not in domain */
#define SDB_CLS_SHARDING_NOT_HASH       -217  /**< sharding type is not hash */
#define SDB_CLS_SPLIT_PERCENT_LOWER     -218  /**< split percent is lower */
#define SDB_TASK_ALREADY_FINISHED       -219  /**< task already finished, can not be caceled */
#define SDB_COLLECTION_LOAD             -220  /**< collection is loading */
#define SDB_LOAD_ROLLBACK               -221  /**< load is error, and rollback */
#define SDB_INVALID_ROUTEID             -222  /**< routeID is different from the local */
#define SDB_DUPLICATED_SERVICE          -223  /**< service already exists */
#define SDB_UTIL_NOT_FIND_FIELD         -224  /**< not find field */
#define SDB_UTIL_CSV_FIELD_END          -225  /**< csv field line end */
#define SDB_MIG_UNKNOW_FILE_TYPE        -226  /**< unknow file type */
#define SDB_RTN_EXPORTCONF_NOT_COMPLETE -227  /**< not all nodes are successful to export configuration */
#define SDB_CLS_NOTP_AND_NODATA         -228  /**< this node is not primary and has no data */
#define SDB_DMS_SECRETVALUE_NOT_SAME    -229  /**< secret value not same with data unit */
#define SDB_PMD_VERSION_ONLY            -230  /**< engine version argument is specified */
#define SDB_SDB_HELP_ONLY               -231  /**< sdb help argument is specified */
#define SDB_SDB_VERSION_ONLY            -232  /**< sdb version argument is specified */
#define SDB_FMP_FUNC_NOT_EXIST          -233  /**< store procedure is not exist */
#define SDB_ILL_RM_SUB_CL               -234  /**< illegal remove sub-collection */
#define SDB_RELINK_SUB_CL               -235  /**< duplicate link sub-collection */
#define SDB_INVALID_MAIN_CL             -236  /**< invalid main-collection */
#define SDB_BOUND_CONFLICT              -237  /**< new boundary is conflict with the existing boundary */
#define SDB_BOUND_INVALID               -238  /**< new boundary is invalid */
#define SDB_HIT_HIGH_WATERMARK          -239  /**< hit the high water mark */
#define SDB_BAR_BACKUP_EXIST            -240  /**< backup already exist */
#define SDB_BAR_BACKUP_NOTEXIST         -241  /**< backup not exist */
#define SDB_INVALID_SUB_CL              -242  /**< invalid sub-collection */
#define SDB_TASK_HAS_CANCELED           -243  /**< task has canceled */
#define SDB_INVALID_MAIN_CL_TYPE        -244  /**< the sharding-type of main-collection must be range */
#define SDB_NO_SHARDINGKEY              -245  /**< there is no valid sharding-key field */
#define SDB_MAIN_CL_OP_ERR              -246  /**< the operation not support on main-collection */
#define SDB_IXM_REDEF                   -247  /**< redefine index */
#define SDB_DMS_CS_DELETING             -248  /**< Deleting the CS */
#define SDB_DMS_REACHED_MAX_NODES       -249  /**< Reached the maximum number of nodes */
#define SDB_CLS_NODE_BSFAULT            -250  /**< the node is business-failures */
#define SDB_CLS_NODE_INFO_EXPIRED       -251  /**< the node info is expired */
#define SDB_CLS_WAIT_SYNC_FAILED        -252  /**< wait secondary nodes sync the operation failed */
#define SDB_DPS_TRANS_DIABLED           -253  /**< transaction is disabled */
#define SDB_DRIVER_DS_RUNOUT            -254  /**< datasource had run out */
#endif /* OSSERR_HPP_ */