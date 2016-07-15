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
#ifndef dmsTRACE_H__
#define dmsTRACE_H__
// Component: dms
#define SDB__DMSCOMPRESSORENTRY_SETCOMPRESSOR              0x100000000016bL
#define SDB__DMSCOMPRESSORENTRY_SETDICTIONARY              0x100000000016cL
#define SDB__DMSCOMPRESSORENTRY_RESET                      0x100000000016dL
#define SDB_DMSCOMPRESS2                                   0x100000000016eL
#define SDB_DMSCOMPRESS                                    0x100000000016fL
#define SDB_DMSUNCOMPRESS                                  0x1000000000170L
#define SDB__DMS_LOBDIRECTOUTBUF_GETALIGNEDTUPLE           0x1000000000171L
#define SDB__DMSSU                                         0x1000000000172L
#define SDB__DMSSU_DESC                                    0x1000000000173L
#define SDB__DMSSU_OPEN                                    0x1000000000174L
#define SDB__DMSSU_CLOSE                                   0x1000000000175L
#define SDB__DMSSU_REMOVE                                  0x1000000000176L
#define SDB__DMSSU__RESETCOLLECTION                        0x1000000000177L
#define SDB__DMSSU_LDEXTA                                  0x1000000000178L
#define SDB__DMSSU_LDEXT                                   0x1000000000179L
#define SDB__DMSSU_INSERTRECORD                            0x100000000017aL
#define SDB__DMSSU_UPDATERECORDS                           0x100000000017bL
#define SDB__DMSSU_DELETERECORDS                           0x100000000017cL
#define SDB__DMSSU_REBUILDINDEXES                          0x100000000017dL
#define SDB__DMSSU_CREATEINDEX                             0x100000000017eL
#define SDB__DMSSU_DROPINDEX                               0x100000000017fL
#define SDB__DMSSU_DROPINDEX1                              0x1000000000180L
#define SDB__DMSSU_COUNTCOLLECTION                         0x1000000000181L
#define SDB__DMSSU_GETCOLLECTIONFLAG                       0x1000000000182L
#define SDB__DMSSU_CHANGECOLLECTIONFLAG                    0x1000000000183L
#define SDB__DMSSU_GETCOLLECTIONATTRIBUTES                 0x1000000000184L
#define SDB__DMSSU_UPDATECOLLECTIONATTRIBUTES              0x1000000000185L
#define SDB__DMSSU_GETSEGEXTENTS                           0x1000000000186L
#define SDB__DMSSU_GETINDEXES                              0x1000000000187L
#define SDB__DMSSU_GETINDEX                                0x1000000000188L
#define SDB__DMSSU_DUMPINFO                                0x1000000000189L
#define SDB__DMSSU_DUMPCLSIMPLE                            0x100000000018aL
#define SDB__DMSSU_DUMPINFO1                               0x100000000018bL
#define SDB__DMSSU_DUMPINFO2                               0x100000000018cL
#define SDB__DMSSU_TOTALSIZE                               0x100000000018dL
#define SDB__DMSSU_TOTALDATAPAGES                          0x100000000018eL
#define SDB__DMSSU_TOTALDATASIZE                           0x100000000018fL
#define SDB__DMSSU_TOTALFREEPAGES                          0x1000000000190L
#define SDB__DMSSU_GETSTATINFO                             0x1000000000191L
#define SDB__DMSSU_TRYTOFLUSH                              0x1000000000192L
#define SDB__DMSROUNIT__INIT                               0x1000000000193L
#define SDB__DMSROUNIT_CLNUP                               0x1000000000194L
#define SDB__DMSROUNIT_OPEN                                0x1000000000195L
#define SDB__DMSROUNIT_IMPMME                              0x1000000000196L
#define SDB__DMSROUNIT_EXPMME                              0x1000000000197L
#define SDB__DMSROUNIT__ALCEXT                             0x1000000000198L
#define SDB__DMSROUNIT__FLSEXT                             0x1000000000199L
#define SDB__DMSROUNIT_FLUSH                               0x100000000019aL
#define SDB__DMSROUNIT_INSRCD                              0x100000000019bL
#define SDB__DMSROUNIT_GETNXTEXTSIZE                       0x100000000019cL
#define SDB__DMSROUNIT_EXPHEAD                             0x100000000019dL
#define SDB__DMSROUNIT_EXPEXT                              0x100000000019eL
#define SDB__DMSROUNIT_VLDHDBUFF                           0x100000000019fL
#define SDB__SDB_DMSCB__LGCSCBNMMAP                        0x10000000001a0L
#define SDB__SDB_DMSCB__CSCBNMINST                         0x10000000001a1L
#define SDB__SDB_DMSCB__CSCBNMREMV                         0x10000000001a2L
#define SDB__SDB_DMSCB__CSCBNMREMVP1                       0x10000000001a3L
#define SDB__SDB_DMSCB__CSCBNMREMVP1CANCEL                 0x10000000001a4L
#define SDB__SDB_DMSCB__CSCBNMREMVP2                       0x10000000001a5L
#define SDB__SDB_DMSCB__CSCBNMMAPCLN                       0x10000000001a6L
#define SDB__SDB_DMSCB_ADDCS                               0x10000000001a7L
#define SDB__SDB_DMSCB_DELCS                               0x10000000001a8L
#define SDB__SDB_DMSCB_DROPCSP1                            0x10000000001a9L
#define SDB__SDB_DMSCB_DROPCSP1CANCEL                      0x10000000001aaL
#define SDB__SDB_DMSCB_DROPCSP2                            0x10000000001abL
#define SDB__SDB_DMSCB_DUMPCLSIMPLE                        0x10000000001acL
#define SDB__SDB_DMSCB_DUMPCSSIMPLE                        0x10000000001adL
#define SDB__SDB_DMSCB_DUMPINFO                            0x10000000001aeL
#define SDB__SDB_DMSCB_DUMPINFO2                           0x10000000001afL
#define SDB__SDB_DMSCB_DUMPINFO3                           0x10000000001b0L
#define SDB__SDB_DMSCB_DUMPINFO4                           0x10000000001b1L
#define SDB__SDB_DMSCB_DISPATCHPAGECLEANSU                 0x10000000001b2L
#define SDB__SDB_DMSCB_JOINPAGECLEANSU                     0x10000000001b3L
#define SDB__SDB_DMSCB__JOINPAGECLEANSU                    0x10000000001b4L
#define SDB__SDB_DMSCB_DISPATCHDICTJOB                     0x10000000001b5L
#define SDB__SDB_DMSCB_PUSHDICTJOB                         0x10000000001b6L
#define SDB__DMSSTORAGELOB_OPEN                            0x10000000001b7L
#define SDB__DMSSTORAGELOB__DELAYOPEN                      0x10000000001b8L
#define SDB__DMSSTORAGELOB__OPENLOB                        0x10000000001b9L
#define SDB__DMSSTORAGELOB_REMOVESTORAGEFILES              0x10000000001baL
#define SDB__DMSSTORAGELOB_GETLOBMETA                      0x10000000001bbL
#define SDB__DMSSTORAGELOB_WRITELOBMETA                    0x10000000001bcL
#define SDB__DMSSTORAGELOB_WRITE                           0x10000000001bdL
#define SDB__DMSSTORAGELOB_UPDATE                          0x10000000001beL
#define SDB__DMSSTORAGELOB_READ                            0x10000000001bfL
#define SDB__DMSSTORAGELOB__ALLOCATEPAGE                   0x10000000001c0L
#define SDB__DMSSTORAGELOB__FILLPAGE                       0x10000000001c1L
#define SDB__DMSSTORAGELOB_REMOVE                          0x10000000001c2L
#define SDB__DMSSTORAGELOB__FIND                           0x10000000001c3L
#define SDB__DMSSTORAGELOB__PUSH2BUCKET                    0x10000000001c4L
#define SDB__DMSSTORAGELOB__ONCREATE                       0x10000000001c5L
#define SDB__DMSSTORAGELOB__ONMAPMETA                      0x10000000001c6L
#define SDB__DMSSTORAGELOB__EXTENDSEGMENTS                 0x10000000001c7L
#define SDB__DMSSTORAGELOB_CALCCOUNT                       0x10000000001c8L
#define SDB__DMSSTORAGELOB_READPAGE                        0x10000000001c9L
#define SDB__DMSSTORAGELOB__REMOVEPAGE                     0x10000000001caL
#define SDB__DMSSTORAGELOB_TRUNCATE                        0x10000000001cbL
#define SDB__DMSSTORAGELOB__ROLLBACK                       0x10000000001ccL
#define SDB__DMSSTORAGELOB_ONFLUSHDIRTY                    0x10000000001cdL
#define SDB__DMSSTORAGELOB_TRYTOFLUSH                      0x10000000001ceL
#define SDB__DMS_LOBDIRECTINBUF_GETALIGNEDTUPLE            0x10000000001cfL
#define SDB__DMS_LOBDIRECTINBUF_CP2USRBUF                  0x10000000001d0L
#define SDB__DMSSTORAGELOADEXT__ALLOCEXTENT                0x10000000001d1L
#define SDB__DMSSTORAGELOADEXT__IMPRTBLOCK                 0x10000000001d2L
#define SDB__DMSSTORAGELOADEXT__LDDATA                     0x10000000001d3L
#define SDB__DMSSTORAGELOADEXT__ROLLEXTENT                 0x10000000001d4L
#define SDB__DMSTMPCB_INIT                                 0x10000000001d5L
#define SDB__DMSTMPCB_RELEASE                              0x10000000001d6L
#define SDB__DMSTMPCB_RESERVE                              0x10000000001d7L
#define SDB_DMSCHKCSNM                                     0x10000000001d8L
#define SDB_DMSCHKFULLCLNM                                 0x10000000001d9L
#define SDB_DMSCHKCLNM                                     0x10000000001daL
#define SDB_DMSCHKINXNM                                    0x10000000001dbL
#define SDB__DMSSMS__RSTMAX                                0x10000000001dcL
#define SDB__DMSSMS_RSVPAGES                               0x10000000001ddL
#define SDB__DMSSMS_RLSPAGES                               0x10000000001deL
#define SDB__DMSSMEMGR_INIT                                0x10000000001dfL
#define SDB__DMSSMEMGR_RSVPAGES                            0x10000000001e0L
#define SDB__DMSSMEMGR_RLSPAGES                            0x10000000001e1L
#define SDB__DMSSMEMGR_DEPOSIT                             0x10000000001e2L
#define SDB__DMS_LOBDIRECTBUF__EXTENDBUF                   0x10000000001e3L
#define SDB_DMSSTORAGELOBDATA_OPEN                         0x10000000001e4L
#define SDB_DMSSTORAGELOBDATA_CLOSE                        0x10000000001e5L
#define SDB_DMSSTORAGELOBDATA_WRITE                        0x10000000001e6L
#define SDB_DMSSTORAGELOBDATA_READ                         0x10000000001e7L
#define SDB_DMSSTORAGELOBDATA_READRAW                      0x10000000001e8L
#define SDB_DMSSTORAGELOBDATA_EXTEND                       0x10000000001e9L
#define SDB_DMSSTORAGELOBDATA__EXTEND                      0x10000000001eaL
#define SDB_DMSSTORAGELOBDATA_REMOVE                       0x10000000001ebL
#define SDB_DMSSTORAGELOBDATA__INITFILEHEADER              0x10000000001ecL
#define SDB_DMSSTORAGELOBDATA__VALIDATEFILE                0x10000000001edL
#define SDB_DMSSTORAGELOBDATA__FETFILEHEADER               0x10000000001eeL
#define SDB_DMSSTORAGELOBDATA_FLUSH                        0x10000000001efL
#define SDB__MBFLAG2STRING                                 0x10000000001f0L
#define SDB__MBATTR2STRING                                 0x10000000001f1L
#define SDB__DMSMBCONTEXT                                  0x10000000001f2L
#define SDB__DMSMBCONTEXT_DESC                             0x10000000001f3L
#define SDB__DMSMBCONTEXT__RESET                           0x10000000001f4L
#define SDB__DMSMBCONTEXT_PAUSE                            0x10000000001f5L
#define SDB__DMSMBCONTEXT_RESUME                           0x10000000001f6L
#define SDB__DMSSTORAGEDATA                                0x10000000001f7L
#define SDB__DMSSTORAGEDATA_DESC                           0x10000000001f8L
#define SDB__DMSSTORAGEDATA_SYNCMEMTOMMAP                  0x10000000001f9L
#define SDB__DMSSTORAGEDATA__ONCREATE                      0x10000000001faL
#define SDB__DMSSTORAGEDATA__INITCOMPRESSORENTRY           0x10000000001fbL
#define SDB__DMSSTORAGEDATA__ONMAPMETA                     0x10000000001fcL
#define SDB__DMSSTORAGEDATA__ONOPENED                      0x10000000001fdL
#define SDB__DMSSTORAGEDATA__ONCLOSED                      0x10000000001feL
#define SDB__DMSSTORAGEDATA__INITMME                       0x10000000001ffL
#define SDB__DMSSTORAGEDATA__LOGDPS                        0x1000000000200L
#define SDB__DMSSTORAGEDATA__LOGDPS1                       0x1000000000201L
#define SDB__DMSSTORAGEDATA__ALLOCATEEXTENT                0x1000000000202L
#define SDB__DMSSTORAGEDATA__FREEEXTENT                    0x1000000000203L
#define SDB__DMSSTORAGEDATA__RESERVEFROMDELETELIST         0x1000000000204L
#define SDB__DMSSTORAGEDATA__TRUNCATECOLLECTION            0x1000000000205L
#define SDB__DMSSTORAGEDATA__TRUNCATECOLLECITONLOADS       0x1000000000206L
#define SDB__DMSSTORAGEDATA__SAVEDELETEDRECORD             0x1000000000207L
#define SDB__DMSSTORAGEDATA__SAVEDELETEDRECORD1            0x1000000000208L
#define SDB__DMSSTORAGEDATA__MAPEXTENT2DELLIST             0x1000000000209L
#define SDB__DMSSTORAGEDATA_ADDEXTENT2META                 0x100000000020aL
#define SDB__DMSSTORAGEDATA_ADDCOLLECTION                  0x100000000020bL
#define SDB__DMSSTORAGEDATA_DROPCOLLECTION                 0x100000000020cL
#define SDB__DMSSTORAGEDATA_TRUNCATECOLLECTION             0x100000000020dL
#define SDB__DMSSTORAGEDATA_TRUNCATECOLLECTIONLOADS        0x100000000020eL
#define SDB__DMSSTORAGEDATA_RENAMECOLLECTION               0x100000000020fL
#define SDB__DMSSTORAGEDATA_FINDCOLLECTION                 0x1000000000210L
#define SDB__DMSSTORAGEDATA_INSERTRECORD                   0x1000000000211L
#define SDB__DMSSTORAGEDATA__EXTENTINSERTRECORD            0x1000000000212L
#define SDB__DMSSTORAGEDATA_DELETERECORD                   0x1000000000213L
#define SDB__DMSSTORAGEDATA__EXTENTREMOVERECORD            0x1000000000214L
#define SDB__DMSSTORAGEDATA_UPDATERECORD                   0x1000000000215L
#define SDB__DMSSTORAGEDATA__EXTENTUPDATERECORD            0x1000000000216L
#define SDB__DMSSTORAGEDATA_FETCH                          0x1000000000217L
#define SDB__DMSSTORAGEDATA_TRYTOFLUSH                     0x1000000000218L
#define SDB__DMSSTORAGEDATA_SETCOMPRESSOR                  0x1000000000219L
#define SDB__DMSSTORAGEDATA_RMCOMPRESSOR                   0x100000000021aL
#define SDB__DMSSTORAGEDATA_DICTPERSIST                    0x100000000021bL
#endif
