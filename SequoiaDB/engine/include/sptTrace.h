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

/* This list file is automatically generated,you shoud NOT modify this file anyway! test comment*/
#ifndef sptTRACE_H__
#define sptTRACE_H__
// Component: spt
#define SDB_SE_GLBSE                                       0x80000000000361L
#define SDB_SE_NEWSCOPE                                    0x80000000000362L
#define SDB_SCOPE_INIT                                     0x80000000000363L
#define SDB_SCOPE_EVALUATE                                 0x80000000000364L
#define SDB_SCOPE_EVALUATE2                                0x80000000000365L
#define SDB_OBJ2BSON                                       0x80000000000366L
#define SDB_BSON_DESTRUCTOR                                0x80000000000367L
#define SDB_BSON_CONSTRUCTOR                               0x80000000000368L
#define SDB_BSON_TO_JSON                                   0x80000000000369L
#define SDB_GLOBAL_PRINT                                   0x8000000000036aL
#define SDB_TRACE_FMT                                      0x8000000000036bL
#define SDB_GLOBAL_HELP                                    0x8000000000036cL
#define SDB_CURSOR_DESTRUCTOR                              0x8000000000036dL
#define SDB_CURSOR_RESV                                    0x8000000000036eL
#define SDB_CURSOR_CONSTRUCTOR                             0x8000000000036fL
#define SDB_CURSOR_NEXT                                    0x80000000000370L
#define SDB_CURSOR_CURRENT                                 0x80000000000371L
#define SDB_CURSOR_CLOSE                                   0x80000000000372L
#define SDB_COUNT_RESV                                     0x80000000000373L
#define SDB_COUNT_CONSTRUCTOR                              0x80000000000374L
#define SDB_COLL_DESTRUCTOR                                0x80000000000375L
#define SDB_COLL_CONSTRUCTOR                               0x80000000000376L
#define SDB_COLL_RAW_FND                                   0x80000000000377L
#define SDB_COLL_INSERT                                    0x80000000000378L
#define SDB_COLL_UPDATE                                    0x80000000000379L
#define SDB_COLL_UPSERT                                    0x8000000000037aL
#define SDB_COLL_REMOVE                                    0x8000000000037bL
#define SDB_COLL_DELETE_LOB                                0x8000000000037cL
#define SDB_COLL_LIST_LOBS                                 0x8000000000037dL
#define SDB_COLL_LIST_LOBPIECES                            0x8000000000037eL
#define SDB_COLL_GET_LOB                                   0x8000000000037fL
#define SDB_COLL_PUT_LOB                                   0x80000000000380L
#define SDB_COLL_EXPLAIN                                   0x80000000000381L
#define SDB_COLL_COUNT                                     0x80000000000382L
#define SDB_COLL_SPLIT                                     0x80000000000383L
#define SDB_COLL_SPLIT_ASYNC                               0x80000000000384L
#define SDB_COLL_CRT_INX                                   0x80000000000385L
#define SDB_COLL_GET_INX                                   0x80000000000386L
#define SDB_COLL_DROP_INX                                  0x80000000000387L
#define SDB_COLL_BULK_INSERT                               0x80000000000388L
#define SDB_COLL_RENM                                      0x80000000000389L
#define SDB_COLL_AGGR                                      0x8000000000038aL
#define SDB_COLL_ATTACHCOLLECTION                          0x8000000000038bL
#define SDB_COLL_DETACHCOLLECTION                          0x8000000000038cL
#define SDB_COLL_TRUNCATE                                  0x8000000000038dL
#define SDB_COLL_CRT_ID_IX                                 0x8000000000038eL
#define SDB_COLL_DROP_ID_IX                                0x8000000000038fL
#define SDB_QUERY_RESV                                     0x80000000000390L
#define SDB_QUERY_CONSTRUCTOR                              0x80000000000391L
#define SDB_RN_DESTRUCTOR                                  0x80000000000392L
#define SB_RG_DESTRUCTOR                                   0x80000000000393L
#define SDB_RG_CONSTRUCTOR                                 0x80000000000394L
#define SDB_RG_GET_MST                                     0x80000000000395L
#define SDB_RG_GET_SLAVE                                   0x80000000000396L
#define SDB_RG_START                                       0x80000000000397L
#define SDB_RG_STOP                                        0x80000000000398L
#define SDB_RG_CRT_NODE                                    0x80000000000399L
#define SDB_RG_RM_NODE                                     0x8000000000039aL
#define SDB_GET_NODE_AND_SETPROPERTY                       0x8000000000039bL
#define SDB_RG_GET_NODE                                    0x8000000000039cL
#define SDB_RG_REELECT                                     0x8000000000039dL
#define SDB_RG_DETACH                                      0x8000000000039eL
#define SDB_RG_ATTACH                                      0x8000000000039fL
#define SDB_CS_DESTRUCTOR                                  0x800000000003a0L
#define SDB_ISSPECCOLLNM                                   0x800000000003a1L
#define SDB_CS_RESV                                        0x800000000003a2L
#define GET_CL_AND_SETPROPERTY                             0x800000000003a3L
#define SDB_CS_GET_CL                                      0x800000000003a4L
#define SB_CS_CRT_CL                                       0x800000000003a5L
#define SDB_CS_DROP_CL                                     0x800000000003a6L
#define SDB_DOMAIN_DESTRUCTOR                              0x800000000003a7L
#define SDB_DOMAIN_ALTER                                   0x800000000003a8L
#define SDB_DOMAIN_LIST_GROUP                              0x800000000003a9L
#define SDB_DOMAIN_LIST_CS                                 0x800000000003aaL
#define SDB_DOMAIN_LIST_CL                                 0x800000000003abL
#define SDB_DC_DESTRUCTOR                                  0x800000000003acL
#define SDB_DC_CONSTRUCTOR                                 0x800000000003adL
#define SDB_GET_DC                                         0x800000000003aeL
#define SDB_DC_CREATEIMAGE                                 0x800000000003afL
#define SDB_DC_REMOVEIMAGE                                 0x800000000003b0L
#define SDB_DC_ATTACHGROUPS                                0x800000000003b1L
#define SDB_DC_DETACHGROUPS                                0x800000000003b2L
#define SDB_DC_ENABLEIMAGE                                 0x800000000003b3L
#define SDB_DC_DISABLEIMAGE                                0x800000000003b4L
#define SDB_DC_ACTIVATE                                    0x800000000003b5L
#define SDB_DC_DEACTIVATE                                  0x800000000003b6L
#define SDB_DC_ENABLEREADONLY                              0x800000000003b7L
#define SDB_DC_DISABLEREADONLY                             0x800000000003b8L
#define SDB_DC_GETDETAIL                                   0x800000000003b9L
#define SDB_DESTRUCTOR                                     0x800000000003baL
#define SDB_ISSPECSNM                                      0x800000000003bbL
#define SDB_SDB_RESV                                       0x800000000003bcL
#define SDB_SDB_CONSTRUCTOR                                0x800000000003bdL
#define SDB_RN_CONNECT                                     0x800000000003beL
#define SDB_RN_START                                       0x800000000003bfL
#define SDB_RN_STOP                                        0x800000000003c0L
#define SDB_SDB_CRT_RG                                     0x800000000003c1L
#define SDB_SDB_CREATE_DOMAIN                              0x800000000003c2L
#define SDB_SDB_DROP_DOMAIN                                0x800000000003c3L
#define SDB_SDB_GET_DOMAIN                                 0x800000000003c4L
#define SDB_SDB_LIST_DOMAINS                               0x800000000003c5L
#define SDB_SDB_CRT_PROCEDURE                              0x800000000003c6L
#define SDB_SDB_RM_PROCEDURE                               0x800000000003c7L
#define SDB_SDB_LIST_PROCEDURES                            0x800000000003c8L
#define SDB_SDB_EVAL                                       0x800000000003c9L
#define SDB_SDB_FLUSH_CONF                                 0x800000000003caL
#define SDB_SDB_RM_RG                                      0x800000000003cbL
#define SDB_SDB_CRT_CATA_RG                                0x800000000003ccL
#define SDB_SDB_CRT_CS                                     0x800000000003cdL
#define SDB_GET_RG_AND_SETPROPERTY                         0x800000000003ceL
#define SDB_SDB_GET_RG                                     0x800000000003cfL
#define GET_CS_AND_SETPROPERTY                             0x800000000003d0L
#define SDB_GET_CS                                         0x800000000003d1L
#define SDB_SDB_DROP_CS                                    0x800000000003d2L
#define SDB_SDB_SNAPSHOT                                   0x800000000003d3L
#define SDB_SDB_RESET_SNAP                                 0x800000000003d4L
#define SDB_SDB_LIST                                       0x800000000003d5L
#define SDB_SDB_START_RG                                   0x800000000003d6L
#define SDB_SDB_CRT_USER                                   0x800000000003d7L
#define SDB_SDB_DROP_USER                                  0x800000000003d8L
#define SDB_SDB_EXEC                                       0x800000000003d9L
#define SDB_SDB_EXECUP                                     0x800000000003daL
#define SDB_SDB_TRACE_ON                                   0x800000000003dbL
#define SDB_SDB_TRACE_RESUME                               0x800000000003dcL
#define SDB_SDB_TRACE_OFF                                  0x800000000003ddL
#define SDB_SDB_TRANS_BEGIN                                0x800000000003deL
#define SDB_SDB_TRANS_COMMIT                               0x800000000003dfL
#define SDB_SDB_TRANS_ROLLBACK                             0x800000000003e0L
#define SDB_SDB_CLOSE                                      0x800000000003e1L
#define SDB_SDB_BACKUP_OFFLINE                             0x800000000003e2L
#define SDB_SDB_LIST_BACKUP                                0x800000000003e3L
#define SDB_SDB_REMOVE_BACKUP                              0x800000000003e4L
#define SDB_SDB_LIST_TASKS                                 0x800000000003e5L
#define SDB_SDB_WAIT_TASKS                                 0x800000000003e6L
#define SDB_SDB_CANCEL_TASK                                0x800000000003e7L
#define SDB_SDB_SET_SESSION_ATTR                           0x800000000003e8L
#define SDB_SDB_MSG                                        0x800000000003e9L
#define SDB_SDB_INVALIDATE_CACHE                           0x800000000003eaL
#define SDB_SDB_FORCE_SESSION                              0x800000000003ebL
#define SDB_SDB_FORCE_STEP_UP                              0x800000000003ecL
#define SDB_SDB_SYNC                                       0x800000000003edL
#define SDB_OBJECTID_DESTRUCTOR                            0x800000000003eeL
#define SDB_OBJECTID_CONSTRUCTOR                           0x800000000003efL
#define SDB_BINDATA_DESTRUCTOR                             0x800000000003f0L
#define SDB_BINDATA_CONSTRUCTOR                            0x800000000003f1L
#define SDB_TIMESTAMP_DESTRUCTOR                           0x800000000003f2L
#define SDB_TIMESTAMP_CONSTRUCTOR                          0x800000000003f3L
#define SDB_REGEX_DESTRUCTOR                               0x800000000003f4L
#define SDB_REGEX_CONSTRUCTOR                              0x800000000003f5L
#define SDB_MINKEY_CONSTRUCTOR                             0x800000000003f6L
#define SDB_MAXKEY_CONSTRUCTOR                             0x800000000003f7L
#define SDB_NUMBERLONG_CONSTRUCTOR                         0x800000000003f8L
#define SDB_SDBDATE_CONSTRUCTOR                            0x800000000003f9L
#endif
