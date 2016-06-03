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
#ifndef catTRACE_H__
#define catTRACE_H__
// Component: cat
#define SDB_CATNODEMGR_INIT                                0x4000003b5L
#define SDB_CATNODEMGR_ACTIVE                              0x4000003b6L
#define SDB_CATNODEMGR_PROCESSMSG                          0x4000003b7L
#define SDB_CATNODEMGR_PRIMARYCHANGE                       0x4000003b8L
#define SDB_CATNODEMGR_GRPREQ                              0x4000003b9L
#define SDB_CATNODEMGR_REGREQ                              0x4000003baL
#define SDB_CATNODEMGR_PCREATEGRP                          0x4000003bbL
#define SDB_CATNODEMGR_CREATENODE                          0x4000003bcL
#define SDB_CATNODEMGR_UPDATENODE                          0x4000003bdL
#define SDB_CATNODEMGR_DELNODE                             0x4000003beL
#define SDB_CATNODEMGR_PREMOVEGRP                          0x4000003bfL
#define SDB_CATNODEMGR_ACTIVEGRP                           0x4000003c0L
#define SDB_CATNODEMGR_READCATACONF                        0x4000003c1L
#define SDB_CATNODEMGR_PARSECATCONF                        0x4000003c2L
#define SDB_CATNODEMGR_SAVEGRPINFO                         0x4000003c3L
#define SDB_CATNODEMGR_GENGROUPINFO                        0x4000003c4L
#define SDB_CATNODEMGR_GETNODEINFOBYCONF                   0x4000003c5L
#define SDB_CATNODEMGR_PARSELINE                           0x4000003c6L
#define SDB_CATNODEMGR_PARSEIDINFO                         0x4000003c7L
#define SDB_CATNODEMGR_GETNODEINFO                         0x4000003c8L
#define SDB_CATNODEMGR_CREATEGRP                           0x4000003c9L
#define SDB_CATNODEMGR_REMOVEGRP                           0x4000003caL
#define SDB_CATALOGCB_INIT                                 0x4000003cbL
#define SDB_CATALOGCB_INSERTGROUPID                        0x4000003ccL
#define SDB_CATALOGCB_REMOVEGROUPID                        0x4000003cdL
#define SDB_CATALOGCB_ACTIVEGROUP                          0x4000003ceL
#define SDB_CATALOGCB_INSERTNODEID                         0x4000003cfL
#define SDB_CATALOGCB_CHECKGROUPACTIVED                    0x4000003d0L
#define SDB_CATALOGCB_GETAGROUPRAND                        0x4000003d1L
#define SDB_CATALOGCB_ALLOCGROUPID                         0x4000003d2L
#define SDB_CATALOGCB_ALLOCCATANODEID                      0x4000003d3L
#define SDB_CATALOGCB_ALLOCNODEID                          0x4000003d4L
#define SDB_CATALOGCB_UPDATEROUTEID                        0x4000003d5L
#define SDB_CATGROUPNAMEVALIDATE                           0x4000003d6L
#define SDB_CATDOMAINOPTIONSEXTRACT                        0x4000003d7L
#define SDB_CATRESOLVECOLLECTIONNAME                       0x4000003d8L
#define SDB_CATQUERYANDGETMORE                             0x4000003d9L
#define SDB_CATGETONEOBJ                                   0x4000003daL
#define SDB_CATGETGROUPOBJ                                 0x4000003dbL
#define SDB_CATGETGROUPOBJ1                                0x4000003dcL
#define SDB_CATGETGROUPOBJ2                                0x4000003ddL
#define SDB_CATGROUPCHECK                                  0x4000003deL
#define SDB_CATSERVICECHECK                                0x4000003dfL
#define SDB_CATGROUPID2NAME                                0x4000003e0L
#define SDB_CATGROUPNAME2ID                                0x4000003e1L
#define SDB_CATGETDOMAINOBJ                                0x4000003e2L
#define SDB_CATDOMAINCHECK                                 0x4000003e3L
#define SDB_CATGETDOMAINGROUPS                             0x4000003e4L
#define SDB_CATGETDOMAINGROUPS1                            0x4000003e5L
#define SDB_CATADDGRP2DOMAIN                               0x4000003e6L
#define SDB_CATDELGRPFROMDOMAIN                            0x4000003e7L
#define SDB_CAATADDCL2CS                                   0x4000003e8L
#define SDB_CATDELCLFROMCS                                 0x4000003e9L
#define SDB_CATRESTORECS                                   0x4000003eaL
#define SDB_CATCHECKSPACEEXIST                             0x4000003ebL
#define SDB_CATREMOVECL                                    0x4000003ecL
#define SDB_CATCHECKCOLLECTIONEXIST                        0x4000003edL
#define SDB_CATUPDATECATALOG                               0x4000003eeL
#define SDB_CATADDTASK                                     0x4000003efL
#define SDB_CATGETTASK                                     0x4000003f0L
#define SDB_CATGETTASKSTATUS                               0x4000003f1L
#define SDB_CATGETMAXTASKID                                0x4000003f2L
#define SDB_CATUPDATETASKSTATUS                            0x4000003f3L
#define SDB_CATREMOVETASK                                  0x4000003f4L
#define SDB_CATREMOVETASK1                                 0x4000003f5L
#define SDB_CATREMOVECLEX                                  0x4000003f6L
#define SDB_CATREMOVECSEX                                  0x4000003f7L
#define SDB_CATPRASEFUNC                                   0x4000003f8L
#define SDB_CATLINKCL                                      0x4000003f9L
#define SDB_CATUNLINKCL                                    0x4000003faL
#define SDB_CATALOGMGR_DROPCS                              0x4000003fbL
#define SDB_CATALOGMGR_CRT_PROCEDURES                      0x4000003fcL
#define SDB_CATALOGMGR_RM_PROCEDURES                       0x4000003fdL
#define SDB_CATALOGMGR_QUERYSPACEINFO                      0x4000003feL
#define SDB_CATALOGMGR_QUERYCATALOG                        0x4000003ffL
#define SDB_CATALOGMGR_DROPCOLLECTION                      0x400000400L
#define SDB_CATALOGMGR_QUERYTASK                           0x400000401L
#define SDB_CATALOGMGR_ALTERCOLLECTION                     0x400000402L
#define SDB_CATALOGMGR__ALTERCOLLECTION                    0x400000403L
#define SDB_CATALOGMGR__ALTERCOLLECTIONOLD                 0x400000404L
#define SDB_CATALOGMGR_CREATECS                            0x400000405L
#define SDB_CATALOGMGR_CREATECL                            0x400000406L
#define SDB_CATALOGMGR_CMDSPLIT                            0x400000407L
#define SDB_CATALOGMGR__CHECKCSOBJ                         0x400000408L
#define SDB_CATALOGMGR__CHECKANDBUILDCATARECORD            0x400000409L
#define SDB_CATALOGMGR__ASSIGNGROUP                        0x40000040aL
#define SDB_CATALOGMGR__CHECKGROUPINDOMAIN                 0x40000040bL
#define SDB_CATALOGMGR__CREATECS                           0x40000040cL
#define SDB_CATALOGMGR_CREATECOLLECTION                    0x40000040dL
#define SDB_CATALOGMGR_BUILDCATALOGRECORD                  0x40000040eL
#define SDB_CATALOGMGR_BUILDINITBOUND                      0x40000040fL
#define SDB_CATALOGMGR_PROCESSMSG                          0x400000410L
#define SDB_CATALOGMGR_PROCESSCOMMANDMSG                   0x400000411L
#define SDB_CATALOGMGR__BUILDHASHBOUND                     0x400000412L
#define SDB_CATALOGMGR_CMDLINKCOLLECTION                   0x400000413L
#define SDB_CATALOGMGR_CMDUNLINKCOLLECTION                 0x400000414L
#define SDB_CATALOGMGR_CREATEDOMAIN                        0x400000415L
#define SDB_CATALOGMGR_DROPDOMAIN                          0x400000416L
#define SDB_CATALOGMGR_ALTERDOMAIN                         0x400000417L
#define SDB_CATALOGMGR__BUILDALTERGROUPS                   0x400000418L
#define SDB_CATALOGMGR__CHOOSEFGROUPOFCL                   0x400000419L
#define SDB_CATALOGMGR_AUTOHASHSPLIT                       0x40000041aL
#define SDB_CATALOGMGR__COMBINEOPTIONS                     0x40000041bL
#define SDB_CATALOGMGR__BUILDALTEROBJWITHMETAANDOBJ        0x40000041cL
#define SDB_CATALOGMGR__GETGROUPSOFCOLLECTIONS             0x40000041dL
#define SDB_CATMAINCT_HANDLEMSG                            0x40000041eL
#define SDB_CATMAINCT_POSTMSG                              0x40000041fL
#define SDB_CATMAINCT_INIT                                 0x400000420L
#define SDB_CATMAINCT__CREATESYSIDX                        0x400000421L
#define SDB_CATMAINCT__CREATESYSCOL                        0x400000422L
#define SDB_CATMAINCT__ENSUREMETADATA                      0x400000423L
#define SDB_CATMAINCT_ACTIVE                               0x400000424L
#define SDB_CATMAINCT_DEACTIVE                             0x400000425L
#define SDB_CATMAINCT_BUILDMSGEVENT                        0x400000426L
#define SDB_CATMAINCT_GETMOREMSG                           0x400000427L
#define SDB_CATMAINCT_KILLCONTEXT                          0x400000428L
#define SDB_CATMAINCT_QUERYMSG                             0x400000429L
#define SDB_CATMAINCT_QUERYREQUEST                         0x40000042aL
#define SDB_CATMAINCT_AUTHCRT                              0x40000042bL
#define SDB_CATMAINCT_AUTHENTICATE                         0x40000042cL
#define SDB_CATMAINCT_AUTHDEL                              0x40000042dL
#define SDB_CATMAINCT_SESSIONINIT                          0x40000042eL
#endif
