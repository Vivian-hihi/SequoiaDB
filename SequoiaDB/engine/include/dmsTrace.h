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
#define SDB_DMSSTORAGELOBDATA_OPEN                         0x1000000000564L
#define SDB_DMSSTORAGELOBDATA_CLOSE                        0x1000000000565L
#define SDB_DMSSTORAGELOBDATA_WRITE                        0x1000000000566L
#define SDB_DMSSTORAGELOBDATA_READ                         0x1000000000567L
#define SDB_DMSSTORAGELOBDATA_READRAW                      0x1000000000568L
#define SDB_DMSSTORAGELOBDATA_EXTEND                       0x1000000000569L
#define SDB_DMSSTORAGELOBDATA__EXTEND                      0x100000000056aL
#define SDB_DMSSTORAGELOBDATA_REMOVE                       0x100000000056bL
#define SDB_DMSSTORAGELOBDATA__INITFILEHEADER              0x100000000056cL
#define SDB_DMSSTORAGELOBDATA__VALIDATEFILE                0x100000000056dL
#define SDB_DMSSTORAGELOBDATA__FETFILEHEADER               0x100000000056eL
#define SDB_DMSSTORAGELOBDATA_FLUSH                        0x100000000056fL
#define SDB_DMSCHKCSNM                                     0x1000000000570L
#define SDB_DMSCHKFULLCLNM                                 0x1000000000571L
#define SDB_DMSCHKCLNM                                     0x1000000000572L
#define SDB_DMSCHKINXNM                                    0x1000000000573L
#define SDB__MBFLAG2STRING                                 0x1000000000574L
#define SDB__MBATTR2STRING                                 0x1000000000575L
#define SDB__DMSMBCONTEXT                                  0x1000000000576L
#define SDB__DMSMBCONTEXT_DESC                             0x1000000000577L
#define SDB__DMSMBCONTEXT__RESET                           0x1000000000578L
#define SDB__DMSMBCONTEXT_PAUSE                            0x1000000000579L
#define SDB__DMSMBCONTEXT_RESUME                           0x100000000057aL
#define SDB__DMSSTORAGEDATA                                0x100000000057bL
#define SDB__DMSSTORAGEDATA_DESC                           0x100000000057cL
#define SDB__DMSSTORAGEDATA_SYNCMEMTOMMAP                  0x100000000057dL
#define SDB__DMSSTORAGEDATA__ONCREATE                      0x100000000057eL
#define SDB__DMSSTORAGEDATA__INITCOMPRESSORENTRY           0x100000000057fL
#define SDB__DMSSTORAGEDATA__ONMAPMETA                     0x1000000000580L
#define SDB__DMSSTORAGEDATA__ONOPENED                      0x1000000000581L
#define SDB__DMSSTORAGEDATA__ONCLOSED                      0x1000000000582L
#define SDB__DMSSTORAGEDATA__INITMME                       0x1000000000583L
#define SDB__DMSSTORAGEDATA__LOGDPS                        0x1000000000584L
#define SDB__DMSSTORAGEDATA__LOGDPS1                       0x1000000000585L
#define SDB__DMSSTORAGEDATA__ALLOCATEEXTENT                0x1000000000586L
#define SDB__DMSSTORAGEDATA__FREEEXTENT                    0x1000000000587L
#define SDB__DMSSTORAGEDATA__RESERVEFROMDELETELIST         0x1000000000588L
#define SDB__DMSSTORAGEDATA__TRUNCATECOLLECTION            0x1000000000589L
#define SDB__DMSSTORAGEDATA__TRUNCATECOLLECITONLOADS       0x100000000058aL
#define SDB__DMSSTORAGEDATA__SAVEDELETEDRECORD             0x100000000058bL
#define SDB__DMSSTORAGEDATA__SAVEDELETEDRECORD1            0x100000000058cL
#define SDB__DMSSTORAGEDATA__MAPEXTENT2DELLIST             0x100000000058dL
#define SDB__DMSSTORAGEDATA_ADDEXTENT2META                 0x100000000058eL
#define SDB__DMSSTORAGEDATA_ADDCOLLECTION                  0x100000000058fL
#define SDB__DMSSTORAGEDATA_DROPCOLLECTION                 0x1000000000590L
#define SDB__DMSSTORAGEDATA_TRUNCATECOLLECTION             0x1000000000591L
#define SDB__DMSSTORAGEDATA_TRUNCATECOLLECTIONLOADS        0x1000000000592L
#define SDB__DMSSTORAGEDATA_RENAMECOLLECTION               0x1000000000593L
#define SDB__DMSSTORAGEDATA_FINDCOLLECTION                 0x1000000000594L
#define SDB__DMSSTORAGEDATA_INSERTRECORD                   0x1000000000595L
#define SDB__DMSSTORAGEDATA__EXTENTINSERTRECORD            0x1000000000596L
#define SDB__DMSSTORAGEDATA_DELETERECORD                   0x1000000000597L
#define SDB__DMSSTORAGEDATA__EXTENTREMOVERECORD            0x1000000000598L
#define SDB__DMSSTORAGEDATA_UPDATERECORD                   0x1000000000599L
#define SDB__DMSSTORAGEDATA__EXTENTUPDATERECORD            0x100000000059aL
#define SDB__DMSSTORAGEDATA_FETCH                          0x100000000059bL
#define SDB__DMSSTORAGEDATA_TRYTOFLUSH                     0x100000000059cL
#define SDB__DMSSTORAGEDATA_SETCOMPRESSOR                  0x100000000059dL
#define SDB__DMSSTORAGEDATA_RMCOMPRESSOR                   0x100000000059eL
#define SDB__DMSSTORAGEDATA_DICTPERSIST                    0x100000000059fL
#define SDB__DMSSMS__RSTMAX                                0x10000000005a0L
#define SDB__DMSSMS_RSVPAGES                               0x10000000005a1L
#define SDB__DMSSMS_RLSPAGES                               0x10000000005a2L
#define SDB__DMSSMEMGR_INIT                                0x10000000005a3L
#define SDB__DMSSMEMGR_RSVPAGES                            0x10000000005a4L
#define SDB__DMSSMEMGR_RLSPAGES                            0x10000000005a5L
#define SDB__DMSSMEMGR_DEPOSIT                             0x10000000005a6L
#define SDB__DMSCOMPRESSORENTRY_SETCOMPRESSOR              0x10000000005a7L
#define SDB__DMSCOMPRESSORENTRY_SETDICTIONARY              0x10000000005a8L
#define SDB__DMSCOMPRESSORENTRY_RESET                      0x10000000005a9L
#define SDB_DMSCOMPRESS2                                   0x10000000005aaL
#define SDB_DMSCOMPRESS                                    0x10000000005abL
#define SDB_DMSUNCOMPRESS                                  0x10000000005acL
#define SDB__DMSTMPCB_INIT                                 0x10000000005adL
#define SDB__DMSTMPCB_RELEASE                              0x10000000005aeL
#define SDB__DMSTMPCB_RESERVE                              0x10000000005afL
#define SDB__DMSSU                                         0x10000000005b0L
#define SDB__DMSSU_DESC                                    0x10000000005b1L
#define SDB__DMSSU_OPEN                                    0x10000000005b2L
#define SDB__DMSSU_CLOSE                                   0x10000000005b3L
#define SDB__DMSSU_REMOVE                                  0x10000000005b4L
#define SDB__DMSSU__RESETCOLLECTION                        0x10000000005b5L
#define SDB__DMSSU_LDEXTA                                  0x10000000005b6L
#define SDB__DMSSU_LDEXT                                   0x10000000005b7L
#define SDB__DMSSU_INSERTRECORD                            0x10000000005b8L
#define SDB__DMSSU_UPDATERECORDS                           0x10000000005b9L
#define SDB__DMSSU_DELETERECORDS                           0x10000000005baL
#define SDB__DMSSU_REBUILDINDEXES                          0x10000000005bbL
#define SDB__DMSSU_CREATEINDEX                             0x10000000005bcL
#define SDB__DMSSU_DROPINDEX                               0x10000000005bdL
#define SDB__DMSSU_DROPINDEX1                              0x10000000005beL
#define SDB__DMSSU_COUNTCOLLECTION                         0x10000000005bfL
#define SDB__DMSSU_GETCOLLECTIONFLAG                       0x10000000005c0L
#define SDB__DMSSU_CHANGECOLLECTIONFLAG                    0x10000000005c1L
#define SDB__DMSSU_GETCOLLECTIONATTRIBUTES                 0x10000000005c2L
#define SDB__DMSSU_UPDATECOLLECTIONATTRIBUTES              0x10000000005c3L
#define SDB__DMSSU_GETSEGEXTENTS                           0x10000000005c4L
#define SDB__DMSSU_GETINDEXES                              0x10000000005c5L
#define SDB__DMSSU_GETINDEX                                0x10000000005c6L
#define SDB__DMSSU_DUMPINFO                                0x10000000005c7L
#define SDB__DMSSU_DUMPCLSIMPLE                            0x10000000005c8L
#define SDB__DMSSU_DUMPINFO1                               0x10000000005c9L
#define SDB__DMSSU_DUMPINFO2                               0x10000000005caL
#define SDB__DMSSU_TOTALSIZE                               0x10000000005cbL
#define SDB__DMSSU_TOTALDATAPAGES                          0x10000000005ccL
#define SDB__DMSSU_TOTALDATASIZE                           0x10000000005cdL
#define SDB__DMSSU_TOTALFREEPAGES                          0x10000000005ceL
#define SDB__DMSSU_GETSTATINFO                             0x10000000005cfL
#define SDB__DMSSU_TRYTOFLUSH                              0x10000000005d0L
#define SDB__DMSROUNIT__INIT                               0x10000000005d1L
#define SDB__DMSROUNIT_CLNUP                               0x10000000005d2L
#define SDB__DMSROUNIT_OPEN                                0x10000000005d3L
#define SDB__DMSROUNIT_IMPMME                              0x10000000005d4L
#define SDB__DMSROUNIT_EXPMME                              0x10000000005d5L
#define SDB__DMSROUNIT__ALCEXT                             0x10000000005d6L
#define SDB__DMSROUNIT__FLSEXT                             0x10000000005d7L
#define SDB__DMSROUNIT_FLUSH                               0x10000000005d8L
#define SDB__DMSROUNIT_INSRCD                              0x10000000005d9L
#define SDB__DMSROUNIT_GETNXTEXTSIZE                       0x10000000005daL
#define SDB__DMSROUNIT_EXPHEAD                             0x10000000005dbL
#define SDB__DMSROUNIT_EXPEXT                              0x10000000005dcL
#define SDB__DMSROUNIT_VLDHDBUFF                           0x10000000005ddL
#define SDB__DMS_LOBDIRECTOUTBUF_GETALIGNEDTUPLE           0x10000000005deL
#define SDB__SDB_DMSCB__LGCSCBNMMAP                        0x10000000005dfL
#define SDB__SDB_DMSCB__CSCBNMINST                         0x10000000005e0L
#define SDB__SDB_DMSCB__CSCBNMREMV                         0x10000000005e1L
#define SDB__SDB_DMSCB__CSCBNMREMVP1                       0x10000000005e2L
#define SDB__SDB_DMSCB__CSCBNMREMVP1CANCEL                 0x10000000005e3L
#define SDB__SDB_DMSCB__CSCBNMREMVP2                       0x10000000005e4L
#define SDB__SDB_DMSCB__CSCBNMMAPCLN                       0x10000000005e5L
#define SDB__SDB_DMSCB_ADDCS                               0x10000000005e6L
#define SDB__SDB_DMSCB_DELCS                               0x10000000005e7L
#define SDB__SDB_DMSCB_DROPCSP1                            0x10000000005e8L
#define SDB__SDB_DMSCB_DROPCSP1CANCEL                      0x10000000005e9L
#define SDB__SDB_DMSCB_DROPCSP2                            0x10000000005eaL
#define SDB__SDB_DMSCB_DUMPCLSIMPLE                        0x10000000005ebL
#define SDB__SDB_DMSCB_DUMPCSSIMPLE                        0x10000000005ecL
#define SDB__SDB_DMSCB_DUMPINFO                            0x10000000005edL
#define SDB__SDB_DMSCB_DUMPINFO2                           0x10000000005eeL
#define SDB__SDB_DMSCB_DUMPINFO3                           0x10000000005efL
#define SDB__SDB_DMSCB_DUMPINFO4                           0x10000000005f0L
#define SDB__SDB_DMSCB_DISPATCHPAGECLEANSU                 0x10000000005f1L
#define SDB__SDB_DMSCB_JOINPAGECLEANSU                     0x10000000005f2L
#define SDB__SDB_DMSCB__JOINPAGECLEANSU                    0x10000000005f3L
#define SDB__SDB_DMSCB_DISPATCHDICTJOB                     0x10000000005f4L
#define SDB__SDB_DMSCB_PUSHDICTJOB                         0x10000000005f5L
#define SDB__DMSSTORAGELOADEXT__ALLOCEXTENT                0x10000000005f6L
#define SDB__DMSSTORAGELOADEXT__IMPRTBLOCK                 0x10000000005f7L
#define SDB__DMSSTORAGELOADEXT__LDDATA                     0x10000000005f8L
#define SDB__DMSSTORAGELOADEXT__ROLLEXTENT                 0x10000000005f9L
#define SDB__DMS_LOBDIRECTBUF__EXTENDBUF                   0x10000000005faL
#define SDB__DMS_LOBDIRECTINBUF_GETALIGNEDTUPLE            0x10000000005fbL
#define SDB__DMS_LOBDIRECTINBUF_CP2USRBUF                  0x10000000005fcL
#define SDB__DMSSTORAGELOB_OPEN                            0x10000000005fdL
#define SDB__DMSSTORAGELOB__DELAYOPEN                      0x10000000005feL
#define SDB__DMSSTORAGELOB__OPENLOB                        0x10000000005ffL
#define SDB__DMSSTORAGELOB_REMOVESTORAGEFILES              0x1000000000600L
#define SDB__DMSSTORAGELOB_GETLOBMETA                      0x1000000000601L
#define SDB__DMSSTORAGELOB_WRITELOBMETA                    0x1000000000602L
#define SDB__DMSSTORAGELOB_WRITE                           0x1000000000603L
#define SDB__DMSSTORAGELOB_UPDATE                          0x1000000000604L
#define SDB__DMSSTORAGELOB_READ                            0x1000000000605L
#define SDB__DMSSTORAGELOB__ALLOCATEPAGE                   0x1000000000606L
#define SDB__DMSSTORAGELOB__FILLPAGE                       0x1000000000607L
#define SDB__DMSSTORAGELOB_REMOVE                          0x1000000000608L
#define SDB__DMSSTORAGELOB__FIND                           0x1000000000609L
#define SDB__DMSSTORAGELOB__PUSH2BUCKET                    0x100000000060aL
#define SDB__DMSSTORAGELOB__ONCREATE                       0x100000000060bL
#define SDB__DMSSTORAGELOB__ONMAPMETA                      0x100000000060cL
#define SDB__DMSSTORAGELOB__EXTENDSEGMENTS                 0x100000000060dL
#define SDB__DMSSTORAGELOB_CALCCOUNT                       0x100000000060eL
#define SDB__DMSSTORAGELOB_READPAGE                        0x100000000060fL
#define SDB__DMSSTORAGELOB__REMOVEPAGE                     0x1000000000610L
#define SDB__DMSSTORAGELOB_TRUNCATE                        0x1000000000611L
#define SDB__DMSSTORAGELOB__ROLLBACK                       0x1000000000612L
#define SDB__DMSSTORAGELOB_ONFLUSHDIRTY                    0x1000000000613L
#define SDB__DMSSTORAGELOB_TRYTOFLUSH                      0x1000000000614L
#endif
