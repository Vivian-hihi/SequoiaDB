/* Error Constants */
var SDB_MAX_ERROR                    = 1024  ;
var SDB_MAX_WARNING                  = 1024  ;
var SDB_OK                           = 0     ;

/* Error Codes */
var SDB_IO                           = -1    ; // IO Exception;
var SDB_OOM                          = -2    ; // Out of Memory;
var SDB_PERM                         = -3    ; // Permission Error;
var SDB_FNE                          = -4    ; // File Not Exist;
var SDB_FE                           = -5    ; // File Exist;
var SDB_INVALIDARG                   = -6    ; // Invalid Argument;
var SDB_INVALIDSIZE                  = -7    ; // Invalid size;
var SDB_INTERRUPT                    = -8    ; // Interrupt;
var SDB_EOF                          = -9    ; // hit end of file;
var SDB_SYS                          = -10   ; // system error;
var SDB_NOSPC                        = -11   ; // has no space;
var SDB_EDU_INVAL_STATUS             = -12   ; // EDU status is not valid;
var SDB_TIMEOUT                      = -13   ; // Timeout error;
var SDB_QUIESCED                     = -14   ; // Database is quiesced;
var SDB_NETWORK                      = -15   ; // Network error;
var SDB_NETWORK_CLOSE                = -16   ; // Network is closed from remote;
var SDB_DATABASE_DOWN                = -17   ; // Database is in shutdown status;
var SDB_APP_FORCED                   = -18   ; // Application is forced;
var SDB_INVALIDPATH                  = -19   ; // Given path is not valid;
var SDB_INVALID_FILE_TYPE            = -20   ; // Unexpected file type specified;
var SDB_DMS_NOSPC                    = -21   ; // There's no space for DMS;
var SDB_DMS_EXIST                    = -22   ; // collection already exist;
var SDB_DMS_NOTEXIST                 = -23   ; // collection does not exist;
var SDB_DMS_RECORD_TOO_BIG           = -24   ; // user record is too big;
var SDB_DMS_RECORD_NOTEXIST          = -25   ; // record does not exist;
var SDB_DMS_OVF_EXIST                = -26   ; // remote overflow record exist;
var SDB_DMS_RECORD_INVALID           = -27   ; // invalid record;
var SDB_DMS_SU_NEED_REORG            = -28   ; // storage unit need reorg;
var SDB_DMS_EOC                      = -29   ; // end of collection;
var SDB_DMS_CONTEXT_IS_OPEN          = -30   ; // context is already opened;
var SDB_DMS_CONTEXT_IS_CLOSE         = -31   ; // context is closed;
var SDB_OPTION_NOT_SUPPORT           = -32   ; // option is not supported yet;
var SDB_DMS_CS_EXIST                 = -33   ; // collection space already exist;
var SDB_DMS_CS_NOTEXIST              = -34   ; // collection space not exist;
var SDB_DMS_INVALID_SU               = -35   ; // storage unit file is invalid;
var SDB_RTN_CONTEXT_NOTEXIST         = -36   ; // context not exist;
var SDB_IXM_MULTIPLE_ARRAY           = -37   ; // more than one field has array;
var SDB_IXM_DUP_KEY                  = -38   ; // duplicate key exist;
var SDB_IXM_KEY_TOO_LARGE            = -39   ; // key is too large;
var SDB_IXM_NOSPC                    = -40   ; // index extent has no space;
var SDB_IXM_KEY_NOTEXIST             = -41   ; // index key not exist;
var SDB_DMS_MAX_INDEX                = -42   ; // hit max number of index;
var SDB_DMS_INIT_INDEX               = -43   ; // failed to initialize index;
var SDB_DMS_COL_DROPPED              = -44   ; // collection is dropped;
var SDB_IXM_IDENTICAL_KEY            = -45   ; // two records get same key and rid;
var SDB_IXM_EXIST                    = -46   ; // duplicate index name;
var SDB_IXM_NOTEXIST                 = -47   ; // index name doesn't exist;
var SDB_IXM_UNEXPECTED_STATUS        = -48   ; // index flag is unexpected;
var SDB_IXM_EOC                      = -49   ; // hit end of index;
var SDB_IXM_DEDUP_BUF_MAX            = -50   ; // hit max of dedup buffer;
var SDB_RTN_INVALID_PREDICATES       = -51   ; // invalid predicates;
var SDB_RTN_INDEX_NOTEXIST           = -52   ; // index is no longer exist;
var SDB_RTN_INVALID_HINT             = -53   ; // index hint is not valid;
var SDB_DMS_NO_MORE_TEMP             = -54   ; // no more temp tables avaliable;
var SDB_DMS_SU_OUTRANGE              = -55   ; // exceed max number of SU;
var SDB_IXM_DROP_ID                  = -56   ; // $id index can't be dropped;
var SDB_DPS_LOG_NOT_IN_BUF           = -57   ; // log was not found in log buf;
var SDB_DPS_LOG_NOT_IN_FILE          = -58   ; // log was not found in log file;
var SDB_PMD_RG_NOT_EXIST             = -59   ; // replication group not exist;
var SDB_PMD_RG_EXIST                 = -60   ; // replication group exist;
var SDB_INVALID_REQID                = -61   ; // invalid request id is received;
var SDB_PMD_SESSION_NOT_EXIST        = -62   ; // session ID does not exist;
var SDB_PMD_FORCE_SYSTEM_EDU         = -63   ; // system edu can't be forced;
var SDB_NOT_CONNECTED                = -64   ; // database is not connected;
var SDB_UNEXPECTED_RESULT            = -65   ; // unexpected result received;
var SDB_CORRUPTED_RECORD             = -66   ; // corrupted record;
var SDB_BACKUP_HAS_ALREADY_START     = -67   ; // backup has already started;
var SDB_BACKUP_NOT_COMPLETE          = -68   ; // backup not completed;
var SDB_RTN_IN_BACKUP                = -69   ; // backup in progress mode;
var SDB_BAR_DAMAGED_BK_FILE          = -70   ; // backup file is damaged;
var SDB_RTN_NO_PRIMARY_FOUND         = -71   ; // there's no primary found;
var SDB_CAT_NODE_NOT_FOUND           = -72   ; // the requested node not exist;
var SDB_PMD_HELP_ONLY                = -73   ; // engine help argument is specified;
var SDB_PMD_CON_INVALID_STATE        = -74   ; // connection state is not valid;
var SDB_CLT_INVALID_HANDLE           = -75   ; // invalid handle;
var SDB_CLT_OBJ_NOT_EXIST            = -76   ; // client object is freed;
var SDB_NET_ALREADY_LISTENED         = -77   ; // transfer has already listened;
var SDB_NET_CANNOT_LISTEN            = -78   ; // can not listen specified addr;
var SDB_NET_CANNOT_CONNECT           = -79   ; // cannot connect to specified addr;
var SDB_NET_NOT_CONNECT              = -80   ; // connection does not exist;
var SDB_NET_SEND_ERR                 = -81   ; // failed to send;
var SDB_NET_TIMER_ID_NOT_FOUND       = -82   ; // timer id not found;
var SDB_NET_ROUTE_NOT_FOUND          = -83   ; // route info not found;
var SDB_NET_BROKEN_MSG               = -84   ; // broken msg;
var SDB_NET_INVALID_HANDLE           = -85   ; // invalid net handle;
var SDB_DMS_INVALID_REORG_FILE       = -86   ; // reorg file is not valid;
var SDB_DMS_REORG_FILE_READONLY      = -87   ; // reorg file is in read only mode;
var SDB_DMS_INVALID_COLLECTION_S     = -88   ; // collection status is not valid;
var SDB_DMS_NOT_IN_REORG             = -89   ; // collection is not in reorg state;
var SDB_REPL_GROUP_NOT_ACTIVE        = -90   ; // replication group is not activated;
var SDB_REPL_INVALID_GROUP_MEMBER    = -91   ; // the member is not in this group;
var SDB_DMS_INCOMPATIBLE_MODE        = -92   ; // collection flag not compatible;
var SDB_DMS_INCOMPATIBLE_VERSION     = -93   ; // stroage unit version not compatible;
var SDB_REPL_LOCAL_G_V_EXPIRED       = -94   ; // local group version is expired;
var SDB_DMS_INVALID_PAGESIZE         = -95   ; // page size is not valid;
var SDB_REPL_REMOTE_G_V_EXPIRED      = -96   ; // remote group version is expired;
var SDB_CLS_VOTE_FAILED              = -97   ; // failed to create a poll;
var SDB_DPS_CORRUPTED_LOG            = -98   ; // log record is corrupted;
var SDB_DPS_LSN_OUTOFRANGE           = -99   ; // given lsn greater than latest;
var SDB_UNKNOWN_MESSAGE              = -100  ; // received unknown mesage;
var SDB_NET_UPDATE_EXISTING_NODE     = -101  ; // updated information is same as old one;
var SDB_CLS_UNKNOW_MSG               = -102  ; // unknow message;
var SDB_CLS_EMPTY_HEAP               = -103  ; // empty heap;
var SDB_CLS_NOT_PRIMARY              = -104  ; // not primary;
var SDB_CLS_NODE_NOT_ENOUGH          = -105  ; // data node not enough;
var SDB_CLS_NO_CATALOG_INFO          = -106  ; // data node have no catalog info;
var SDB_CLS_DATA_NODE_CAT_VER_OLD    = -107  ; // data node catlog version old;
var SDB_CLS_COORD_NODE_CAT_VER_OLD   = -108  ; // coord node catalog version old;
var SDB_CLS_INVALID_GROUP_NUM        = -109  ; // exceds the max group size;
var SDB_CLS_SYNC_FAILED              = -110  ; // failed to sync log;
var SDB_CLS_REPLAY_LOG_FAILED        = -111  ; // failed to replay log;
var SDB_REST_EHS                     = -112  ; // error http struct;
var SDB_CLS_CONSULT_FAILED           = -113  ; // failed to negotiate;
var SDB_DPS_MOVE_FAILED              = -114  ; // failed to change dps metadata;
var SDB_DMS_CORRUPTED_SME            = -115  ; // SME is corrupted;
var SDB_APP_INTERRUPT                = -116  ; // application is interrupted;
var SDB_APP_DISCONNECT               = -117  ; // application is disconnected;
var SDB_OSS_CCE                      = -118  ; // character encoding errors;
var SDB_COORD_QUERY_FAILED           = -119  ; // get failed msg from the node;
var SDB_CLS_BUFFER_FULL              = -120  ; // buffer array is full;
var SDB_RTN_SUBCONTEXT_CONFLICT      = -121  ; // sub context is conflict;
var SDB_COORD_QUERY_EOC              = -122  ; // coord received EOC message;
var SDB_DPS_FILE_SIZE_NOT_SAME       = -123  ; // DPS file size are not the same;
var SDB_DPS_FILE_NOT_RECOGNISE       = -124  ; // dps file not recognise;
var SDB_OSS_NORES                    = -125  ; // no resource;
var SDB_DPS_INVALID_LSN              = -126  ; // invalid lsn;
var SDB_OSS_NPIPE_DATA_TOO_BIG       = -127  ; // data to pipe is too big;
var SDB_CAT_AUTH_FAILED              = -128  ; // catalog auth failed;
var SDB_CLS_FULL_SYNC                = -129  ; // node is full sync;
var SDB_CAT_ASSIGN_NODE_FAILED       = -130  ; // catnode failed assign data-node;
var SDB_PHP_DRIVER_INTERNAL_ERROR    = -131  ; // php driver internal error;
var SDB_COORD_SEND_MSG_FAILED        = -132  ; // failed to send the message;
var SDB_CAT_NO_NODEGROUP_INFO        = -133  ; // there is no node-group register in catalogue-node;
var SDB_COORD_REMOTE_DISC            = -134  ; // remote-node disconnected;
var SDB_CAT_NO_MATCH_CATALOG         = -135  ; // couldn't find the match catalogue-info;
var SDB_CLS_UPDATE_CAT_FAILED        = -136  ; // update catalog failed;
var SDB_COORD_UNKNOWN_OP_REQ         = -137  ; // received invalid request which opcode is unknowned;
var SDB_COOR_NO_NODEGROUP_INFO       = -138  ; // coord couldn't find the group-info in local;
var SDB_DMS_CORRUPTED_EXTENT         = -139  ; // dms extent is corrupted;
var SDBCM_FAIL                       = -140  ; // remote cluster manage failed;
var SDBCM_STOP_PART                  = -141  ; // remote engines have been stopped partially;
var SDBCM_SVC_STARTING               = -142  ; // sdb service is starting;
var SDBCM_SVC_STARTED                = -143  ; // sdb service has already been started;
var SDBCM_SVC_RESTARTING             = -144  ; // sdb service is restarting;
var SDBCM_NODE_EXISTED               = -145  ; // sdb node is already existed;
var SDBCM_NODE_NOTEXISTED            = -146  ; // sdb node is not existed;
var SDB_LOCK_FAILED                  = -147  ; // unable to lock;
var SDB_DMS_STATE_NOT_COMPATIBLE     = -148  ; // dms state is not compatible with current command;
var SDB_REBUILD_HAS_ALREADY_START    = -149  ; // rebuild has already started;
var SDB_RTN_IN_REBUILD               = -150  ; // rebuild in progress mode;
var SDB_RTN_COORD_CACHE_EMPTY        = -151  ; // there is no data in the coord-node's cache ;
var SDB_SPT_EVAL_FAIL                = -152  ; // errors occur during the process of evalution;
var SDB_CAT_GRP_EXIST                = -153  ; // group already exist;
var SDB_CLS_GRP_NOT_EXIST            = -154  ; // group does not exist;
var SDB_CLS_NODE_NOT_EXIST           = -155  ; // node does not exist;
var SDB_CM_RUN_NODE_FAILED           = -156  ; // failed to start the node;
var SDB_CM_CONFIG_CONFLICTS          = -157  ; // node'configure conflicts;
var SDB_CLS_EMPTY_GROUP              = -158  ; // group is empty;
var SDB_RTN_COORD_ONLY               = -159  ; // The operation is for coord node only;
var SDB_CM_OP_NODE_FAILED            = -160  ; // failed to operate on node only;
var SDB_RTN_MUTEX_JOB_EXIST          = -161  ; // The mutex job already exist;
var SDB_RTN_JOB_NOT_EXIST            = -162  ; // The specified job does not exist;
var SDB_CAT_CORRUPTION               = -163  ; // The catalog information is corrupted;
var SDB_IXM_DROP_SHARD               = -164  ; // $shard index can't be dropped;
var SDB_RTN_CMD_NO_NODE_AUTH         = -165  ; // The command can't be run in the node;
var SDB_RTN_CMD_NO_SERVICE_AUTH      = -166  ; // The command can't be run in the serice plane;
var SDB_CLS_NO_GROUP_INFO            = -167  ; // The group info not exist;
var SDB_CLS_GROUP_NAME_CONFLICT      = -168  ; // Group name is conflict;
var SDB_COLLECTION_NOTSHARD          = -169  ; // The collection is not sharded;
var SDB_INVALID_SHARDINGKEY          = -170  ; // The record does not contains valid sharding key;
var SDB_TASK_EXIST                   = -171  ; // A task that already exists does not compatible with the new task;
var SDB_CL_NOT_EXIST_ON_GROUP        = -172  ; // The collection does not exists on the specified group;
var SDB_CAT_TASK_NOTFOUND            = -173  ; // The specified task does not exist;
var SDB_MULTI_SHARDING_KEY           = -174  ; // The record contains more than one sharding key;
var SDB_CLS_MUTEX_TASK_EXIST         = -175  ; // The mutex task already exist;
var SDB_CLS_BAD_SPLIT_KEY            = -176  ; // The split key is not valid or not in the source group;
var SDB_SHARD_KEY_NOT_IN_UNIQUE_KEY  = -177  ; // The unique index must include all fields in sharding key;
var SDB_UPDATE_SHARD_KEY             = -178  ; // Sharding key cannot be updated;
var SDB_AUTH_AUTHORITY_FORBIDDEN     = -179  ; // authority is forbidden;
var SDB_CAT_NO_ADDR_LIST             = -180  ; // There is no catalog address specified by user;
var SDB_CURRENT_RECORD_DELETED       = -181  ; // current record has been deleted;
var SDB_QGM_MATCH_NONE               = -182  ; // search condition cannot match any records;
var SDB_IXM_REORG_DONE               = -183  ; // index page is reorged and the pos got different lchild;
var SDB_RTN_DUPLICATE_FIELDNAME      = -184  ; // There are duplicate field name exists in the record;
var SDB_QGM_MAX_NUM_RECORD           = -185  ; // Too many records to be inserted at once;
var SDB_QGM_MERGE_JOIN_EQONLY        = -186  ; // Sort-Merge Join only supports equal predicates;
var SDB_PD_TRACE_IS_STARTED          = -187  ; // Trace is already started;
var SDB_PD_TRACE_HAS_NO_BUFFER       = -188  ; // Trace buffer does not exist;
var SDB_PD_TRACE_FILE_INVALID        = -189  ; // Trace file is not valid;
var SDB_DPS_TRANS_LOCK_INCOMPATIBLE  = -190  ; // transaction-lock is incompatible;
var SDB_DPS_TRANS_DOING_ROLLBACK     = -191  ; // system is doing rollback;
var SDB_MIG_IMP_BAD_RECORD           = -192  ; // Bad record when doing sdb import ;
var SDB_QGM_REPEAT_VAR_NAME          = -193  ; // repeat var name was found;
var SDB_QGM_AMBIGUOUS_FIELD          = -194  ; // column field is ambiguous;
var SDB_SQL_SYNTAX_ERROR             = -195  ; // have an error in sql syntax;
var SDB_DPS_TRANS_NO_TRANS           = -196  ; // invalid transactional operation;
var SDB_DPS_TRANS_APPEND_TO_WAIT     = -197  ; // append to lock-wait-queue;
var SDB_DMS_DELETING                 = -198  ; // record is deleting;
var SDB_DMS_INVALID_INDEXCB          = -199  ; // index is dropped or invalid;
var SDB_COORD_RECREATE_CATALOG       = -200  ; // repeat to create catalog-group;
var SDB_UTIL_PARSE_JSON_INVALID      = -201  ; // parse json error;
var SDB_UTIL_PARSE_CSV_INVALID       = -202  ; // parse CSV error;
var SDB_DPS_LOG_FILE_OUT_OF_SIZE     = -203  ; // log file is out of size;
var SDB_CATA_RM_NODE_FORBIDDEN       = -204  ; // can not remove the only node in the group;
var SDB_CATA_FAILED_TO_CLEANUP       = -205  ; // need to manually complete the cleanup;
var SDB_CATA_RM_CATA_FORBIDDEN       = -206  ; // can not remove node or group of catalog when other group exist;
var SDB_CAT_GRP_NOT_EXIST            = -207  ; // group is not exist;
var SDB_CAT_RM_GRP_FORBIDDEN         = -208  ; // can not remove the group with data in it;
var SDB_MIG_END_OF_QUEUE             = -209  ; // end of queue;
var SDB_COORD_SPLIT_NO_SHDIDX        = -210  ; // collection has not sharding index, can not split by percent;
var SDB_FIELD_NOT_EXIST              = -211  ; // the param field not exist;
var SDB_TOO_MANY_TRACE_BP            = -212  ; // too many trace break points are specified;
var SDB_BUSY_PREFETCHER              = -213  ; // prefetchers are all busy;
var SDB_CAT_DOMAIN_NOT_EXIST         = -214  ; // domain not exist;
var SDB_CAT_DOMAIN_EXIST             = -215  ; // domain already exist;
var SDB_CAT_GROUP_NOT_IN_DOMAIN      = -216  ; // group is not in domain;
var SDB_CLS_SHARDING_NOT_HASH        = -217  ; // sharding type is not hash;
var SDB_CLS_SPLIT_PERCENT_LOWER      = -218  ; // split percent is lower;
var SDB_TASK_ALREADY_FINISHED        = -219  ; // task already finished, can not be caceled;
var SDB_COLLECTION_LOAD              = -220  ; // collection is loading;
var SDB_LOAD_ROLLBACK                = -221  ; // load is error, and rollback;
var SDB_INVALID_ROUTEID              = -222  ; // routeID is different from the local;
var SDB_DUPLICATED_SERVICE           = -223  ; // service already exists;
var SDB_UTIL_NOT_FIND_FIELD          = -224  ; // not find field;
var SDB_UTIL_CSV_FIELD_END           = -225  ; // csv field line end;
var SDB_MIG_UNKNOW_FILE_TYPE         = -226  ; // unknow file type;
var SDB_RTN_EXPORTCONF_NOT_COMPLETE  = -227  ; // not all nodes are successful to export configuration;
var SDB_CLS_NOTP_AND_NODATA          = -228  ; // this node is not primary and has no data;
var SDB_DMS_SECRETVALUE_NOT_SAME     = -229  ; // secret value not same with data unit;
var SDB_PMD_VERSION_ONLY             = -230  ; // engine version argument is specified;
var SDB_SDB_HELP_ONLY                = -231  ; // sdb help argument is specified;
var SDB_SDB_VERSION_ONLY             = -232  ; // sdb version argument is specified;
var SDB_FMP_FUNC_NOT_EXIST           = -233  ; // store procedure is not exist;
var SDB_ILL_RM_SUB_CL                = -234  ; // illegal remove sub-collection;
var SDB_RELINK_SUB_CL                = -235  ; // duplicate link sub-collection;
var SDB_INVALID_MAIN_CL              = -236  ; // invalid main-collection;
var SDB_BOUND_CONFLICT               = -237  ; // new boundary is conflict with the existing boundary;
var SDB_BOUND_INVALID                = -238  ; // new boundary is invalid;
var SDB_HIT_HIGH_WATERMARK           = -239  ; // hit the high water mark;
var SDB_BAR_BACKUP_EXIST             = -240  ; // backup already exist;
var SDB_BAR_BACKUP_NOTEXIST          = -241  ; // backup not exist;
var SDB_INVALID_SUB_CL               = -242  ; // invalid sub-collection;
var SDB_TASK_HAS_CANCELED            = -243  ; // task has canceled;
var SDB_INVALID_MAIN_CL_TYPE         = -244  ; // the sharding-type of main-collection must be range;
var SDB_NO_SHARDINGKEY               = -245  ; // there is no valid sharding-key field;
var SDB_MAIN_CL_OP_ERR               = -246  ; // the operation not support on main-collection;
var SDB_IXM_REDEF                    = -247  ; // redefine index;
var SDB_DMS_CS_DELETING              = -248  ; // Deleting the CS;
var SDB_DMS_REACHED_MAX_NODES        = -249  ; // Reached the maximum number of nodes;
var SDB_CLS_NODE_BSFAULT             = -250  ; // the node is business-failures;
var SDB_CLS_NODE_INFO_EXPIRED        = -251  ; // the node info is expired;
var SDB_CLS_WAIT_SYNC_FAILED         = -252  ; // wait secondary nodes sync the operation failed;
var SDB_DPS_TRANS_DIABLED            = -253  ; // transaction is disabled;
var SDB_DRIVER_DS_RUNOUT             = -254  ; // datasource had run out;
var SDB_TOO_MANY_OPEN_FD             = -255  ; // too many open file description;
var SDB_DOMAIN_IS_OCCUPIED           = -256  ; // Domain is not empty;
var SDB_REST_RECV_SIZE               = -257  ; // Rest recv size greater than max size;
var SDB_DRIVER_BSON_ERROR            = -258  ; // Wrong data for building bson;
var SDB_OUT_OF_BOUND                 = -259  ; // out of bound;
var SDB_REST_COMMON_UNKNOW           = -260  ; // rest common unknow;
var SDB_BUT_FAILED_ON_DATA           = -261  ; // successfully done on catalog, but sth wrong happened when did it on data group.;
var SDB_CAT_NO_GROUP_IN_DOMAIN       = -262  ; // domain does not have any groups at all;
var SDB_OM_PASSWD_CHANGE_SUGGUEST    = -263  ; // suggest sdb om's user to change the passwd;
var SDB_COORD_NOT_ALL_DONE           = -264  ; // not all nodes returned ok;
var SDB_OMA_DIFF_VER_AGT_IS_RUNNING  = -265  ; // different version agent has been running;
var SDB_OM_TASK_NOT_EXIST            = -266  ; // task is not exist;
var SDB_OM_TASK_ROLLBACK             = -267  ; // task is rolling back;
var SDB_LOB_SEQUENCE_NOT_EXIST       = -268  ; // LOB sequence does not exist;
var SDB_LOB_IS_UNDER_CRT             = -269  ; // LOB creation is not completed;
var SDB_MIG_DATA_NON_UTF             = -270  ; // the data is not utf8;
var SDB_OMA_TASK_FAIL                = -271  ; // task is failing;

function getErr (errCode) {
   var errDesp = [ 
                   "Succeed",
                   "IO Exception",
                   "Out of Memory",
                   "Permission Error",
                   "File Not Exist",
                   "File Exist",
                   "Invalid Argument",
                   "Invalid size",
                   "Interrupt",
                   "hit end of file",
                   "system error",
                   "has no space",
                   "EDU status is not valid",
                   "Timeout error",
                   "Database is quiesced",
                   "Network error",
                   "Network is closed from remote",
                   "Database is in shutdown status",
                   "Application is forced",
                   "Given path is not valid",
                   "Unexpected file type specified",
                   "There's no space for DMS",
                   "collection already exist",
                   "collection does not exist",
                   "user record is too big",
                   "record does not exist",
                   "remote overflow record exist",
                   "invalid record",
                   "storage unit need reorg",
                   "end of collection",
                   "context is already opened",
                   "context is closed",
                   "option is not supported yet",
                   "collection space already exist",
                   "collection space not exist",
                   "storage unit file is invalid",
                   "context not exist",
                   "more than one field has array",
                   "duplicate key exist",
                   "key is too large",
                   "index extent has no space",
                   "index key not exist",
                   "hit max number of index",
                   "failed to initialize index",
                   "collection is dropped",
                   "two records get same key and rid",
                   "duplicate index name",
                   "index name doesn't exist",
                   "index flag is unexpected",
                   "hit end of index",
                   "hit max of dedup buffer",
                   "invalid predicates",
                   "index is no longer exist",
                   "index hint is not valid",
                   "no more temp tables avaliable",
                   "exceed max number of SU",
                   "$id index can't be dropped",
                   "log was not found in log buf",
                   "log was not found in log file",
                   "replication group not exist",
                   "replication group exist",
                   "invalid request id is received",
                   "session ID does not exist",
                   "system edu can't be forced",
                   "database is not connected",
                   "unexpected result received",
                   "corrupted record",
                   "backup has already started",
                   "backup not completed",
                   "backup in progress mode",
                   "backup file is damaged",
                   "there's no primary found",
                   "the requested node not exist",
                   "engine help argument is specified",
                   "connection state is not valid",
                   "invalid handle",
                   "client object is freed",
                   "transfer has already listened",
                   "can not listen specified addr",
                   "cannot connect to specified addr",
                   "connection does not exist",
                   "failed to send",
                   "timer id not found",
                   "route info not found",
                   "broken msg",
                   "invalid net handle",
                   "reorg file is not valid",
                   "reorg file is in read only mode",
                   "collection status is not valid",
                   "collection is not in reorg state",
                   "replication group is not activated",
                   "the member is not in this group",
                   "collection flag not compatible",
                   "stroage unit version not compatible",
                   "local group version is expired",
                   "page size is not valid",
                   "remote group version is expired",
                   "failed to create a poll",
                   "log record is corrupted",
                   "given lsn greater than latest",
                   "received unknown mesage",
                   "updated information is same as old one",
                   "unknow message",
                   "empty heap",
                   "not primary",
                   "data node not enough",
                   "data node have no catalog info",
                   "data node catlog version old",
                   "coord node catalog version old",
                   "exceds the max group size",
                   "failed to sync log",
                   "failed to replay log",
                   "error http struct",
                   "failed to negotiate",
                   "failed to change dps metadata",
                   "SME is corrupted",
                   "application is interrupted",
                   "application is disconnected",
                   "character encoding errors",
                   "get failed msg from the node",
                   "buffer array is full",
                   "sub context is conflict",
                   "coord received EOC message",
                   "DPS file size are not the same",
                   "dps file not recognise",
                   "no resource",
                   "invalid lsn",
                   "data to pipe is too big",
                   "catalog auth failed",
                   "node is full sync",
                   "catnode failed assign data-node",
                   "php driver internal error",
                   "failed to send the message",
                   "there is no node-group register in catalogue-node",
                   "remote-node disconnected",
                   "couldn't find the match catalogue-info",
                   "update catalog failed",
                   "received invalid request which opcode is unknowned",
                   "coord couldn't find the group-info in local",
                   "dms extent is corrupted",
                   "remote cluster manage failed",
                   "remote engines have been stopped partially",
                   "sdb service is starting",
                   "sdb service has already been started",
                   "sdb service is restarting",
                   "sdb node is already existed",
                   "sdb node is not existed",
                   "unable to lock",
                   "dms state is not compatible with current command",
                   "rebuild has already started",
                   "rebuild in progress mode",
                   "there is no data in the coord-node's cache ",
                   "errors occur during the process of evalution",
                   "group already exist",
                   "group does not exist",
                   "node does not exist",
                   "failed to start the node",
                   "node'configure conflicts",
                   "group is empty",
                   "The operation is for coord node only",
                   "failed to operate on node only",
                   "The mutex job already exist",
                   "The specified job does not exist",
                   "The catalog information is corrupted",
                   "$shard index can't be dropped",
                   "The command can't be run in the node",
                   "The command can't be run in the serice plane",
                   "The group info not exist",
                   "Group name is conflict",
                   "The collection is not sharded",
                   "The record does not contains valid sharding key",
                   "A task that already exists does not compatible with the new task",
                   "The collection does not exists on the specified group",
                   "The specified task does not exist",
                   "The record contains more than one sharding key",
                   "The mutex task already exist",
                   "The split key is not valid or not in the source group",
                   "The unique index must include all fields in sharding key",
                   "Sharding key cannot be updated",
                   "authority is forbidden",
                   "There is no catalog address specified by user",
                   "current record has been deleted",
                   "search condition cannot match any records",
                   "index page is reorged and the pos got different lchild",
                   "There are duplicate field name exists in the record",
                   "Too many records to be inserted at once",
                   "Sort-Merge Join only supports equal predicates",
                   "Trace is already started",
                   "Trace buffer does not exist",
                   "Trace file is not valid",
                   "transaction-lock is incompatible",
                   "system is doing rollback",
                   "Bad record when doing sdb import ",
                   "repeat var name was found",
                   "column field is ambiguous",
                   "have an error in sql syntax",
                   "invalid transactional operation",
                   "append to lock-wait-queue",
                   "record is deleting",
                   "index is dropped or invalid",
                   "repeat to create catalog-group",
                   "parse json error",
                   "parse CSV error",
                   "log file is out of size",
                   "can not remove the only node in the group",
                   "need to manually complete the cleanup",
                   "can not remove node or group of catalog when other group exist",
                   "group is not exist",
                   "can not remove the group with data in it",
                   "end of queue",
                   "collection has not sharding index, can not split by percent",
                   "the param field not exist",
                   "too many trace break points are specified",
                   "prefetchers are all busy",
                   "domain not exist",
                   "domain already exist",
                   "group is not in domain",
                   "sharding type is not hash",
                   "split percent is lower",
                   "task already finished, can not be caceled",
                   "collection is loading",
                   "load is error, and rollback",
                   "routeID is different from the local",
                   "service already exists",
                   "not find field",
                   "csv field line end",
                   "unknow file type",
                   "not all nodes are successful to export configuration",
                   "this node is not primary and has no data",
                   "secret value not same with data unit",
                   "engine version argument is specified",
                   "sdb help argument is specified",
                   "sdb version argument is specified",
                   "store procedure is not exist",
                   "illegal remove sub-collection",
                   "duplicate link sub-collection",
                   "invalid main-collection",
                   "new boundary is conflict with the existing boundary",
                   "new boundary is invalid",
                   "hit the high water mark",
                   "backup already exist",
                   "backup not exist",
                   "invalid sub-collection",
                   "task has canceled",
                   "the sharding-type of main-collection must be range",
                   "there is no valid sharding-key field",
                   "the operation not support on main-collection",
                   "redefine index",
                   "Deleting the CS",
                   "Reached the maximum number of nodes",
                   "the node is business-failures",
                   "the node info is expired",
                   "wait secondary nodes sync the operation failed",
                   "transaction is disabled",
                   "datasource had run out",
                   "too many open file description",
                   "Domain is not empty",
                   "Rest recv size greater than max size",
                   "Wrong data for building bson",
                   "out of bound",
                   "rest common unknow",
                   "successfully done on catalog, but sth wrong happened when did it on data group.",
                   "domain does not have any groups at all",
                   "suggest sdb om's user to change the passwd",
                   "not all nodes returned ok",
                   "different version agent has been running",
                   "task is not exist",
                   "task is rolling back",
                   "LOB sequence does not exist",
                   "LOB creation is not completed",
                   "the data is not utf8",
                   "task is failing"
   ]; 
   var index = -errCode ;
   if ( index < 0 || index >= errDesp.length ) 
      return "unknown error"
   return errDesp[index] ;
}
