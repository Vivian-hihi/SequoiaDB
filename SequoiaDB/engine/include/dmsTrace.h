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
#define SDB_DMSCHKCSNM                                     0x100000000025eL
#define SDB_DMSCHKFULLCLNM                                 0x100000000025fL
#define SDB_DMSCHKCLNM                                     0x1000000000260L
#define SDB_DMSCHKINXNM                                    0x1000000000261L
#define SDB__SDB_DMSCB__LGCSCBNMMAP                        0x1000000000262L
#define SDB__SDB_DMSCB__CSCBNMINST                         0x1000000000263L
#define SDB__SDB_DMSCB__CSCBNMREMV                         0x1000000000264L
#define SDB__SDB_DMSCB__CSCBNMREMVP1                       0x1000000000265L
#define SDB__SDB_DMSCB__CSCBNMREMVP1CANCEL                 0x1000000000266L
#define SDB__SDB_DMSCB__CSCBNMREMVP2                       0x1000000000267L
#define SDB__SDB_DMSCB__CSCBNMMAPCLN                       0x1000000000268L
#define SDB__SDB_DMSCB_ADDCS                               0x1000000000269L
#define SDB__SDB_DMSCB_DELCS                               0x100000000026aL
#define SDB__SDB_DMSCB_DROPCSP1                            0x100000000026bL
#define SDB__SDB_DMSCB_DROPCSP1CANCEL                      0x100000000026cL
#define SDB__SDB_DMSCB_DROPCSP2                            0x100000000026dL
#define SDB__SDB_DMSCB_DUMPINFO                            0x100000000026eL
#define SDB__SDB_DMSCB_DUMPINFO2                           0x100000000026fL
#define SDB__SDB_DMSCB_DUMPINFO3                           0x1000000000270L
#define SDB__SDB_DMSCB_DUMPINFO4                           0x1000000000271L
#define SDB__SDB_DMSCB_DISPATCHPAGECLEANSU                 0x1000000000272L
#define SDB__SDB_DMSCB_JOINPAGECLEANSU                     0x1000000000273L
#define SDB__SDB_DMSCB__JOINPAGECLEANSU                    0x1000000000274L
#define SDB__SDB_DMSCB_DISPATCHDICTCREATECL                0x1000000000275L
#define SDB__SDB_DMSCB_PUSHTODICTCREATECLLIST              0x1000000000276L
#define SDB__SDB_DMSCB_POPFROMDICTCREATECLLIST             0x1000000000277L
#define SDB__SDB_DMSCB_SKIPCURRENTDICTCREATECL             0x1000000000278L
#define SDB__DMS_LOBDIRECTBUF__EXTENDBUF                   0x1000000000279L
#define SDB__DMS_LOBDIRECTINBUF_GETALIGNEDTUPLE            0x100000000027aL
#define SDB__DMS_LOBDIRECTINBUF_CP2USRBUF                  0x100000000027bL
#define SDB__DMS_LOBDIRECTOUTBUF_GETALIGNEDTUPLE           0x100000000027cL
#define SDB__DMSROUNIT__INIT                               0x100000000027dL
#define SDB__DMSROUNIT_CLNUP                               0x100000000027eL
#define SDB__DMSROUNIT_OPEN                                0x100000000027fL
#define SDB__DMSROUNIT_IMPMME                              0x1000000000280L
#define SDB__DMSROUNIT_EXPMME                              0x1000000000281L
#define SDB__DMSROUNIT__ALCEXT                             0x1000000000282L
#define SDB__DMSROUNIT__FLSEXT                             0x1000000000283L
#define SDB__DMSROUNIT_FLUSH                               0x1000000000284L
#define SDB__DMSROUNIT_INSRCD                              0x1000000000285L
#define SDB__DMSROUNIT_GETNXTEXTSIZE                       0x1000000000286L
#define SDB__DMSROUNIT_EXPHEAD                             0x1000000000287L
#define SDB__DMSROUNIT_EXPEXT                              0x1000000000288L
#define SDB__DMSROUNIT_VLDHDBUFF                           0x1000000000289L
#define SDB__DMSSMS__RSTMAX                                0x100000000028aL
#define SDB__DMSSMS_RSVPAGES                               0x100000000028bL
#define SDB__DMSSMS_RLSPAGES                               0x100000000028cL
#define SDB__DMSSMEMGR_INIT                                0x100000000028dL
#define SDB__DMSSMEMGR_RSVPAGES                            0x100000000028eL
#define SDB__DMSSMEMGR_RLSPAGES                            0x100000000028fL
#define SDB__DMSSMEMGR_DEPOSIT                             0x1000000000290L
#define SDB__MBFLAG2STRING                                 0x1000000000291L
#define SDB__MBATTR2STRING                                 0x1000000000292L
#define SDB__DMSMBCONTEXT                                  0x1000000000293L
#define SDB__DMSMBCONTEXT_DESC                             0x1000000000294L
#define SDB__DMSMBCONTEXT__RESET                           0x1000000000295L
#define SDB__DMSMBCONTEXT_PAUSE                            0x1000000000296L
#define SDB__DMSMBCONTEXT_RESUME                           0x1000000000297L
#define SDB__DMSSTORAGEDATA                                0x1000000000298L
#define SDB__DMSSTORAGEDATA_DESC                           0x1000000000299L
#define SDB__DMSSTORAGEDATA_SYNCMEMTOMMAP                  0x100000000029aL
#define SDB__DMSSTORAGEDATA__ONCREATE                      0x100000000029bL
#define SDB__DMSSTORAGEDATA__LOADCLDICTTOCACHE             0x100000000029cL
#define SDB__DMSSTORAGEDATA__ONMAPMETA                     0x100000000029dL
#define SDB__DMSSTORAGEDATA__ONOPENED                      0x100000000029eL
#define SDB__DMSSTORAGEDATA__ONCLOSED                      0x100000000029fL
#define SDB__DMSSTORAGEDATA__INITMME                       0x10000000002a0L
#define SDB__DMSSTORAGEDATA__LOGDPS                        0x10000000002a1L
#define SDB__DMSSTORAGEDATA__LOGDPS1                       0x10000000002a2L
#define SDB__DMSSTORAGEDATA__ALLOCATEEXTENT                0x10000000002a3L
#define SDB__DMSSTORAGEDATA__FREEEXTENT                    0x10000000002a4L
#define SDB__DMSSTORAGEDATA__RESERVEFROMDELETELIST         0x10000000002a5L
#define SDB__DMSSTORAGEDATA__TRUNCATECOLLECTION            0x10000000002a6L
#define SDB__DMSSTORAGEDATA__TRUNCATECOLLECITONLOADS       0x10000000002a7L
#define SDB__DMSSTORAGEDATA__SAVEDELETEDRECORD             0x10000000002a8L
#define SDB__DMSSTORAGEDATA__SAVEDELETEDRECORD1            0x10000000002a9L
#define SDB__DMSSTORAGEDATA__MAPEXTENT2DELLIST             0x10000000002aaL
#define SDB__DMSSTORAGEDATA_ADDEXTENT2META                 0x10000000002abL
#define SDB__DMSSTORAGEDATA_ADDCOLLECTION                  0x10000000002acL
#define SDB__DMSSTORAGEDATA_DROPCOLLECTION                 0x10000000002adL
#define SDB__DMSSTORAGEDATA_TRUNCATECOLLECTION             0x10000000002aeL
#define SDB__DMSSTORAGEDATA_TRUNCATECOLLECTIONLOADS        0x10000000002afL
#define SDB__DMSSTORAGEDATA_RENAMECOLLECTION               0x10000000002b0L
#define SDB__DMSSTORAGEDATA_FINDCOLLECTION                 0x10000000002b1L
#define SDB__DMSSTORAGEDATA_INSERTRECORD                   0x10000000002b2L
#define SDB__DMSSTORAGEDATA__EXTENTINSERTRECORD            0x10000000002b3L
#define SDB__DMSSTORAGEDATA_DELETERECORD                   0x10000000002b4L
#define SDB__DMSSTORAGEDATA__EXTENTREMOVERECORD            0x10000000002b5L
#define SDB__DMSSTORAGEDATA_UPDATERECORD                   0x10000000002b6L
#define SDB__DMSSTORAGEDATA__EXTENTUPDATERECORD            0x10000000002b7L
#define SDB__DMSSTORAGEDATA_FETCH                          0x10000000002b8L
#define SDB__DMSSTORAGEDATA_TRYTOFLUSH                     0x10000000002b9L
#define SDB__DMSDICTCONTEXT_RESET                          0x10000000002baL
#define SDB__DMSDICTCONTEXT_GETCOMPRESSOR                  0x10000000002bbL
#define SDB__DMSDICTCONTEXT_RELEASECOMPRESSOR              0x10000000002bcL
#define SDB__DMSDICTCONTEXT_SETDICT                        0x10000000002bdL
#define SDB__DMSSTORAGEDATA_ADDDICTTOCACHE                 0x10000000002beL
#define SDB__DMSSTORAGEDATA_SAVECLDICT                     0x10000000002bfL
#define SDB__DMSDICTCACHE_CLEARCACHE                       0x10000000002c0L
#define SDB__DMSSTORAGELOADEXT__ALLOCEXTENT                0x10000000002c1L
#define SDB__DMSSTORAGELOADEXT__IMPRTBLOCK                 0x10000000002c2L
#define SDB__DMSSTORAGELOADEXT__LDDATA                     0x10000000002c3L
#define SDB__DMSSTORAGELOADEXT__ROLLEXTENT                 0x10000000002c4L
#define SDB__DMSSTORAGELOB_OPEN                            0x10000000002c5L
#define SDB__DMSSTORAGELOB__DELAYOPEN                      0x10000000002c6L
#define SDB__DMSSTORAGELOB__OPENLOB                        0x10000000002c7L
#define SDB__DMSSTORAGELOB_REMOVESTORAGEFILES              0x10000000002c8L
#define SDB__DMSSTORAGELOB_GETLOBMETA                      0x10000000002c9L
#define SDB__DMSSTORAGELOB_WRITELOBMETA                    0x10000000002caL
#define SDB__DMSSTORAGELOB_WRITE                           0x10000000002cbL
#define SDB__DMSSTORAGELOB_UPDATE                          0x10000000002ccL
#define SDB__DMSSTORAGELOB_READ                            0x10000000002cdL
#define SDB__DMSSTORAGELOB__ALLOCATEPAGE                   0x10000000002ceL
#define SDB__DMSSTORAGELOB__FILLPAGE                       0x10000000002cfL
#define SDB__DMSSTORAGELOB_REMOVE                          0x10000000002d0L
#define SDB__DMSSTORAGELOB__FIND                           0x10000000002d1L
#define SDB__DMSSTORAGELOB__PUSH2BUCKET                    0x10000000002d2L
#define SDB__DMSSTORAGELOB__ONCREATE                       0x10000000002d3L
#define SDB__DMSSTORAGELOB__ONMAPMETA                      0x10000000002d4L
#define SDB__DMSSTORAGELOB__EXTENDSEGMENTS                 0x10000000002d5L
#define SDB__DMSSTORAGELOB_READPAGE                        0x10000000002d6L
#define SDB__DMSSTORAGELOB__REMOVEPAGE                     0x10000000002d7L
#define SDB__DMSSTORAGELOB_TRUNCATE                        0x10000000002d8L
#define SDB__DMSSTORAGELOB__ROLLBACK                       0x10000000002d9L
#define SDB__DMSSTORAGELOB_TRYTOFLUSH                      0x10000000002daL
#define SDB_DMSSTORAGELOBDATA_OPEN                         0x10000000002dbL
#define SDB_DMSSTORAGELOBDATA__REOPEN                      0x10000000002dcL
#define SDB_DMSSTORAGELOBDATA_CLOSE                        0x10000000002ddL
#define SDB_DMSSTORAGELOBDATA_WRITE                        0x10000000002deL
#define SDB_DMSSTORAGELOBDATA_READ                         0x10000000002dfL
#define SDB_DMSSTORAGELOBDATA_READRAW                      0x10000000002e0L
#define SDB_DMSSTORAGELOBDATA_EXTEND                       0x10000000002e1L
#define SDB_DMSSTORAGELOBDATA__EXTEND                      0x10000000002e2L
#define SDB_DMSSTORAGELOBDATA_REMOVE                       0x10000000002e3L
#define SDB_DMSSTORAGELOBDATA__INITFILEHEADER              0x10000000002e4L
#define SDB_DMSSTORAGELOBDATA__VALIDATEFILE                0x10000000002e5L
#define SDB_DMSSTORAGELOBDATA__FETFILEHEADER               0x10000000002e6L
#define SDB_DMSSTORAGELOBDATA_FLUSH                        0x10000000002e7L
#define SDB__DMSSU                                         0x10000000002e8L
#define SDB__DMSSU_DESC                                    0x10000000002e9L
#define SDB__DMSSU_OPEN                                    0x10000000002eaL
#define SDB__DMSSU_CLOSE                                   0x10000000002ebL
#define SDB__DMSSU_REMOVE                                  0x10000000002ecL
#define SDB__DMSSU__RESETCOLLECTION                        0x10000000002edL
#define SDB__DMSSU_LDEXTA                                  0x10000000002eeL
#define SDB__DMSSU_LDEXT                                   0x10000000002efL
#define SDB__DMSSU_INSERTRECORD                            0x10000000002f0L
#define SDB__DMSSU_UPDATERECORDS                           0x10000000002f1L
#define SDB__DMSSU_DELETERECORDS                           0x10000000002f2L
#define SDB__DMSSU_REBUILDINDEXES                          0x10000000002f3L
#define SDB__DMSSU_CREATEINDEX                             0x10000000002f4L
#define SDB__DMSSU_DROPINDEX                               0x10000000002f5L
#define SDB__DMSSU_DROPINDEX1                              0x10000000002f6L
#define SDB__DMSSU_COUNTCOLLECTION                         0x10000000002f7L
#define SDB__DMSSU_GETCOLLECTIONFLAG                       0x10000000002f8L
#define SDB__DMSSU_CHANGECOLLECTIONFLAG                    0x10000000002f9L
#define SDB__DMSSU_GETCOLLECTIONATTRIBUTES                 0x10000000002faL
#define SDB__DMSSU_UPDATECOLLECTIONATTRIBUTES              0x10000000002fbL
#define SDB__DMSSU_GETSEGEXTENTS                           0x10000000002fcL
#define SDB__DMSSU_GETINDEXES                              0x10000000002fdL
#define SDB__DMSSU_GETINDEX                                0x10000000002feL
#define SDB__DMSSU_DUMPINFO                                0x10000000002ffL
#define SDB__DMSSU_DUMPINFO1                               0x1000000000300L
#define SDB__DMSSU_DUMPINFO2                               0x1000000000301L
#define SDB__DMSSU_TOTALSIZE                               0x1000000000302L
#define SDB__DMSSU_TOTALDATAPAGES                          0x1000000000303L
#define SDB__DMSSU_TOTALDATASIZE                           0x1000000000304L
#define SDB__DMSSU_TOTALFREEPAGES                          0x1000000000305L
#define SDB__DMSSU_GETSTATINFO                             0x1000000000306L
#define SDB__DMSSU_TRYTOFLUSH                              0x1000000000307L
#define SDB__DMSTMPCB_INIT                                 0x1000000000308L
#define SDB__DMSTMPCB_RELEASE                              0x1000000000309L
#define SDB__DMSTMPCB_RESERVE                              0x100000000030aL
#endif
