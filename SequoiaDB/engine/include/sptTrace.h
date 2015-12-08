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
#define SDB_OBJ2BSON                                       0x80000000000881L
#define SDB_BSON_DESTRUCTOR                                0x80000000000882L
#define SDB_BSON_CONSTRUCTOR                               0x80000000000883L
#define SDB_BSON_TO_JSON                                   0x80000000000884L
#define SDB_GLOBAL_PRINT                                   0x80000000000885L
#define SDB_TRACE_FMT                                      0x80000000000886L
#define SDB_GLOBAL_HELP                                    0x80000000000887L
#define SDB_CURSOR_DESTRUCTOR                              0x80000000000888L
#define SDB_CURSOR_RESV                                    0x80000000000889L
#define SDB_CURSOR_CONSTRUCTOR                             0x8000000000088aL
#define SDB_CURSOR_NEXT                                    0x8000000000088bL
#define SDB_CURSOR_CURRENT                                 0x8000000000088cL
#define SDB_CURSOR_CLOSE                                   0x8000000000088dL
#define SDB_COUNT_RESV                                     0x8000000000088eL
#define SDB_COUNT_CONSTRUCTOR                              0x8000000000088fL
#define SDB_COLL_DESTRUCTOR                                0x80000000000890L
#define SDB_COLL_CONSTRUCTOR                               0x80000000000891L
#define SDB_COLL_RAW_FND                                   0x80000000000892L
#define SDB_COLL_INSERT                                    0x80000000000893L
#define SDB_COLL_UPDATE                                    0x80000000000894L
#define SDB_COLL_UPSERT                                    0x80000000000895L
#define SDB_COLL_REMOVE                                    0x80000000000896L
#define SDB_COLL_DELETE_LOB                                0x80000000000897L
#define SDB_COLL_LIST_LOBS                                 0x80000000000898L
#define SDB_COLL_LIST_LOBPIECES                            0x80000000000899L
#define SDB_COLL_GET_LOB                                   0x8000000000089aL
#define SDB_COLL_PUT_LOB                                   0x8000000000089bL
#define SDB_COLL_EXPLAIN                                   0x8000000000089cL
#define SDB_COLL_COUNT                                     0x8000000000089dL
#define SDB_COLL_SPLIT                                     0x8000000000089eL
#define SDB_COLL_SPLIT_ASYNC                               0x8000000000089fL
#define SDB_COLL_CRT_INX                                   0x800000000008a0L
#define SDB_COLL_GET_INX                                   0x800000000008a1L
#define SDB_COLL_DROP_INX                                  0x800000000008a2L
#define SDB_COLL_BULK_INSERT                               0x800000000008a3L
#define SDB_COLL_RENM                                      0x800000000008a4L
#define SDB_COLL_AGGR                                      0x800000000008a5L
#define SDB_COLL_ATTACHCOLLECTION                          0x800000000008a6L
#define SDB_COLL_DETACHCOLLECTION                          0x800000000008a7L
#define SDB_COLL_TRUNCATE                                  0x800000000008a8L
#define SDB_COLL_CRT_ID_IX                                 0x800000000008a9L
#define SDB_COLL_DROP_ID_IX                                0x800000000008aaL
#define SDB_QUERY_RESV                                     0x800000000008abL
#define SDB_QUERY_CONSTRUCTOR                              0x800000000008acL
#define SDB_RN_DESTRUCTOR                                  0x800000000008adL
#define SB_RG_DESTRUCTOR                                   0x800000000008aeL
#define SDB_RG_CONSTRUCTOR                                 0x800000000008afL
#define SDB_RG_GET_MST                                     0x800000000008b0L
#define SDB_RG_GET_SLAVE                                   0x800000000008b1L
#define SDB_RG_START                                       0x800000000008b2L
#define SDB_RG_STOP                                        0x800000000008b3L
#define SDB_RG_CRT_NODE                                    0x800000000008b4L
#define SDB_RG_RM_NODE                                     0x800000000008b5L
#define SDB_GET_NODE_AND_SETPROPERTY                       0x800000000008b6L
#define SDB_RG_GET_NODE                                    0x800000000008b7L
#define SDB_RG_REELECT                                     0x800000000008b8L
#define SDB_RG_DETACH                                      0x800000000008b9L
#define SDB_RG_ATTACH                                      0x800000000008baL
#define SDB_CS_DESTRUCTOR                                  0x800000000008bbL
#define SDB_ISSPECCOLLNM                                   0x800000000008bcL
#define SDB_CS_RESV                                        0x800000000008bdL
#define GET_CL_AND_SETPROPERTY                             0x800000000008beL
#define SDB_CS_GET_CL                                      0x800000000008bfL
#define SB_CS_CRT_CL                                       0x800000000008c0L
#define SDB_CS_DROP_CL                                     0x800000000008c1L
#define SDB_DOMAIN_DESTRUCTOR                              0x800000000008c2L
#define SDB_DOMAIN_ALTER                                   0x800000000008c3L
#define SDB_DOMAIN_LIST_GROUP                              0x800000000008c4L
#define SDB_DOMAIN_LIST_CS                                 0x800000000008c5L
#define SDB_DOMAIN_LIST_CL                                 0x800000000008c6L
#define SDB_DC_DESTRUCTOR                                  0x800000000008c7L
#define SDB_DC_CONSTRUCTOR                                 0x800000000008c8L
#define SDB_GET_DC                                         0x800000000008c9L
#define SDB_DC_CREATEIMAGE                                 0x800000000008caL
#define SDB_DC_REMOVEIMAGE                                 0x800000000008cbL
#define SDB_DC_ATTACHGROUPS                                0x800000000008ccL
#define SDB_DC_DETACHGROUPS                                0x800000000008cdL
#define SDB_DC_ENABLEIMAGE                                 0x800000000008ceL
#define SDB_DC_DISABLEIMAGE                                0x800000000008cfL
#define SDB_DC_ACTIVATE                                    0x800000000008d0L
#define SDB_DC_DEACTIVATE                                  0x800000000008d1L
#define SDB_DC_ENABLEREADONLY                              0x800000000008d2L
#define SDB_DC_DISABLEREADONLY                             0x800000000008d3L
#define SDB_DC_GETDETAIL                                   0x800000000008d4L
#define SDB_DESTRUCTOR                                     0x800000000008d5L
#define SDB_ISSPECSNM                                      0x800000000008d6L
#define SDB_SDB_RESV                                       0x800000000008d7L
#define SDB_SDB_CONSTRUCTOR                                0x800000000008d8L
#define SDB_RN_CONNECT                                     0x800000000008d9L
#define SDB_RN_START                                       0x800000000008daL
#define SDB_RN_STOP                                        0x800000000008dbL
#define SDB_SDB_CRT_RG                                     0x800000000008dcL
#define SDB_SDB_CREATE_DOMAIN                              0x800000000008ddL
#define SDB_SDB_DROP_DOMAIN                                0x800000000008deL
#define SDB_SDB_GET_DOMAIN                                 0x800000000008dfL
#define SDB_SDB_LIST_DOMAINS                               0x800000000008e0L
#define SDB_SDB_CRT_PROCEDURE                              0x800000000008e1L
#define SDB_SDB_RM_PROCEDURE                               0x800000000008e2L
#define SDB_SDB_LIST_PROCEDURES                            0x800000000008e3L
#define SDB_SDB_EVAL                                       0x800000000008e4L
#define SDB_SDB_FLUSH_CONF                                 0x800000000008e5L
#define SDB_SDB_RM_RG                                      0x800000000008e6L
#define SDB_SDB_CRT_CATA_RG                                0x800000000008e7L
#define SDB_SDB_CRT_CS                                     0x800000000008e8L
#define SDB_GET_RG_AND_SETPROPERTY                         0x800000000008e9L
#define SDB_SDB_GET_RG                                     0x800000000008eaL
#define GET_CS_AND_SETPROPERTY                             0x800000000008ebL
#define SDB_GET_CS                                         0x800000000008ecL
#define SDB_SDB_DROP_CS                                    0x800000000008edL
#define SDB_SDB_SNAPSHOT                                   0x800000000008eeL
#define SDB_SDB_RESET_SNAP                                 0x800000000008efL
#define SDB_SDB_LIST                                       0x800000000008f0L
#define SDB_SDB_START_RG                                   0x800000000008f1L
#define SDB_SDB_CRT_USER                                   0x800000000008f2L
#define SDB_SDB_DROP_USER                                  0x800000000008f3L
#define SDB_SDB_EXEC                                       0x800000000008f4L
#define SDB_SDB_EXECUP                                     0x800000000008f5L
#define SDB_SDB_TRACE_ON                                   0x800000000008f6L
#define SDB_SDB_TRACE_RESUME                               0x800000000008f7L
#define SDB_SDB_TRACE_OFF                                  0x800000000008f8L
#define SDB_SDB_TRANS_BEGIN                                0x800000000008f9L
#define SDB_SDB_TRANS_COMMIT                               0x800000000008faL
#define SDB_SDB_TRANS_ROLLBACK                             0x800000000008fbL
#define SDB_SDB_CLOSE                                      0x800000000008fcL
#define SDB_SDB_BACKUP_OFFLINE                             0x800000000008fdL
#define SDB_SDB_LIST_BACKUP                                0x800000000008feL
#define SDB_SDB_REMOVE_BACKUP                              0x800000000008ffL
#define SDB_SDB_LIST_TASKS                                 0x80000000000900L
#define SDB_SDB_WAIT_TASKS                                 0x80000000000901L
#define SDB_SDB_CANCEL_TASK                                0x80000000000902L
#define SDB_SDB_SET_SESSION_ATTR                           0x80000000000903L
#define SDB_SDB_MSG                                        0x80000000000904L
#define SDB_SDB_INVALIDATE_CACHE                           0x80000000000905L
#define SDB_SDB_FORCE_SESSION                              0x80000000000906L
#define SDB_SDB_FORCE_STEP_UP                              0x80000000000907L
#define SDB_SDB_SYNC                                       0x80000000000908L
#define SDB_OBJECTID_DESTRUCTOR                            0x80000000000909L
#define SDB_OBJECTID_CONSTRUCTOR                           0x8000000000090aL
#define SDB_BINDATA_DESTRUCTOR                             0x8000000000090bL
#define SDB_BINDATA_CONSTRUCTOR                            0x8000000000090cL
#define SDB_TIMESTAMP_DESTRUCTOR                           0x8000000000090dL
#define SDB_TIMESTAMP_CONSTRUCTOR                          0x8000000000090eL
#define SDB_REGEX_DESTRUCTOR                               0x8000000000090fL
#define SDB_REGEX_CONSTRUCTOR                              0x80000000000910L
#define SDB_MINKEY_CONSTRUCTOR                             0x80000000000911L
#define SDB_MAXKEY_CONSTRUCTOR                             0x80000000000912L
#define SDB_NUMBERLONG_CONSTRUCTOR                         0x80000000000913L
#define SDB_SDBDATE_CONSTRUCTOR                            0x80000000000914L
#define SDB_SE_GLBSE                                       0x80000000000915L
#define SDB_SE_NEWSCOPE                                    0x80000000000916L
#define SDB_SCOPE_INIT                                     0x80000000000917L
#define SDB_SCOPE_EVALUATE                                 0x80000000000918L
#define SDB_SCOPE_EVALUATE2                                0x80000000000919L
#endif
