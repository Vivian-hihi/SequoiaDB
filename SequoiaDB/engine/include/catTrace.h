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
#define SDB_CATALOGCB_INIT                                 0x40000046aL
#define SDB_CATALOGCB_INSERTGROUPID                        0x40000046bL
#define SDB_CATALOGCB_REMOVEGROUPID                        0x40000046cL
#define SDB_CATALOGCB_ACTIVEGROUP                          0x40000046dL
#define SDB_CATALOGCB_INSERTNODEID                         0x40000046eL
#define SDB_CATALOGCB_CHECKGROUPACTIVED                    0x40000046fL
#define SDB_CATALOGCB_GETAGROUPRAND                        0x400000470L
#define SDB_CATALOGCB_ALLOCGROUPID                         0x400000471L
#define SDB_CATALOGCB_ALLOCCATANODEID                      0x400000472L
#define SDB_CATALOGCB_ALLOCNODEID                          0x400000473L
#define SDB_CATALOGCB_UPDATEROUTEID                        0x400000474L
#define SDB_CATGROUPNAMEVALIDATE                           0x400000475L
#define SDB_CATDOMAINOPTIONSEXTRACT                        0x400000476L
#define SDB_CATRESOLVECOLLECTIONNAME                       0x400000477L
#define SDB_CATQUERYANDGETMORE                             0x400000478L
#define SDB_CATGETONEOBJ                                   0x400000479L
#define SDB_CATGETGROUPOBJ                                 0x40000047aL
#define SDB_CATGETGROUPOBJ1                                0x40000047bL
#define SDB_CATGETGROUPOBJ2                                0x40000047cL
#define SDB_CATGROUPCHECK                                  0x40000047dL
#define SDB_CATSERVICECHECK                                0x40000047eL
#define SDB_CATGROUPID2NAME                                0x40000047fL
#define SDB_CATGROUPNAME2ID                                0x400000480L
#define SDB_CATGETDOMAINOBJ                                0x400000481L
#define SDB_CATDOMAINCHECK                                 0x400000482L
#define SDB_CATGETDOMAINGROUPS                             0x400000483L
#define SDB_CATGETDOMAINGROUPS1                            0x400000484L
#define SDB_CATADDGRP2DOMAIN                               0x400000485L
#define SDB_CATDELGRPFROMDOMAIN                            0x400000486L
#define SDB_CAATADDCL2CS                                   0x400000487L
#define SDB_CATDELCLFROMCS                                 0x400000488L
#define SDB_CATRESTORECS                                   0x400000489L
#define SDB_CATCHECKSPACEEXIST                             0x40000048aL
#define SDB_CATREMOVECL                                    0x40000048bL
#define SDB_CATCHECKCOLLECTIONEXIST                        0x40000048cL
#define SDB_CATUPDATECATALOG                               0x40000048dL
#define SDB_CATADDTASK                                     0x40000048eL
#define SDB_CATGETTASK                                     0x40000048fL
#define SDB_CATGETTASKSTATUS                               0x400000490L
#define SDB_CATGETMAXTASKID                                0x400000491L
#define SDB_CATUPDATETASKSTATUS                            0x400000492L
#define SDB_CATREMOVETASK                                  0x400000493L
#define SDB_CATREMOVETASK1                                 0x400000494L
#define SDB_CATREMOVECLEX                                  0x400000495L
#define SDB_CATREMOVECSEX                                  0x400000496L
#define SDB_CATPRASEFUNC                                   0x400000497L
#define SDB_CATLINKCL                                      0x400000498L
#define SDB_CATUNLINKCL                                    0x400000499L
#define SDB_CATMAINCT_HANDLEMSG                            0x40000049aL
#define SDB_CATMAINCT_POSTMSG                              0x40000049bL
#define SDB_CATMAINCT_INIT                                 0x40000049cL
#define SDB_CATMAINCT__CREATESYSIDX                        0x40000049dL
#define SDB_CATMAINCT__CREATESYSCOL                        0x40000049eL
#define SDB_CATMAINCT__ENSUREMETADATA                      0x40000049fL
#define SDB_CATMAINCT_ACTIVE                               0x4000004a0L
#define SDB_CATMAINCT_DEACTIVE                             0x4000004a1L
#define SDB_CATMAINCT_BUILDMSGEVENT                        0x4000004a2L
#define SDB_CATMAINCT_GETMOREMSG                           0x4000004a3L
#define SDB_CATMAINCT_KILLCONTEXT                          0x4000004a4L
#define SDB_CATMAINCT_QUERYMSG                             0x4000004a5L
#define SDB_CATMAINCT_QUERYREQUEST                         0x4000004a6L
#define SDB_CATMAINCT_AUTHCRT                              0x4000004a7L
#define SDB_CATMAINCT_AUTHENTICATE                         0x4000004a8L
#define SDB_CATMAINCT_AUTHDEL                              0x4000004a9L
#define SDB_CATMAINCT_SESSIONINIT                          0x4000004aaL
#define SDB_CATNODEMGR_INIT                                0x4000004abL
#define SDB_CATNODEMGR_ACTIVE                              0x4000004acL
#define SDB_CATNODEMGR_PROCESSMSG                          0x4000004adL
#define SDB_CATNODEMGR_PRIMARYCHANGE                       0x4000004aeL
#define SDB_CATNODEMGR_GRPREQ                              0x4000004afL
#define SDB_CATNODEMGR_REGREQ                              0x4000004b0L
#define SDB_CATNODEMGR_PCREATEGRP                          0x4000004b1L
#define SDB_CATNODEMGR_CREATENODE                          0x4000004b2L
#define SDB_CATNODEMGR_UPDATENODE                          0x4000004b3L
#define SDB_CATNODEMGR_DELNODE                             0x4000004b4L
#define SDB_CATNODEMGR_PREMOVEGRP                          0x4000004b5L
#define SDB_CATNODEMGR_ACTIVEGRP                           0x4000004b6L
#define SDB_CATNODEMGR_READCATACONF                        0x4000004b7L
#define SDB_CATNODEMGR_PARSECATCONF                        0x4000004b8L
#define SDB_CATNODEMGR_SAVEGRPINFO                         0x4000004b9L
#define SDB_CATNODEMGR_GENGROUPINFO                        0x4000004baL
#define SDB_CATNODEMGR_GETNODEINFOBYCONF                   0x4000004bbL
#define SDB_CATNODEMGR_PARSELINE                           0x4000004bcL
#define SDB_CATNODEMGR_PARSEIDINFO                         0x4000004bdL
#define SDB_CATNODEMGR_GETNODEINFO                         0x4000004beL
#define SDB_CATNODEMGR_CREATEGRP                           0x4000004bfL
#define SDB_CATNODEMGR_REMOVEGRP                           0x4000004c0L
#define SDB_CATALOGMGR_DROPCS                              0x4000004c1L
#define SDB_CATALOGMGR_CRT_PROCEDURES                      0x4000004c2L
#define SDB_CATALOGMGR_RM_PROCEDURES                       0x4000004c3L
#define SDB_CATALOGMGR_QUERYSPACEINFO                      0x4000004c4L
#define SDB_CATALOGMGR_QUERYCATALOG                        0x4000004c5L
#define SDB_CATALOGMGR_DROPCOLLECTION                      0x4000004c6L
#define SDB_CATALOGMGR_QUERYTASK                           0x4000004c7L
#define SDB_CATALOGMGR_ALTERCOLLECTION                     0x4000004c8L
#define SDB_CATALOGMGR__ALTERCOLLECTION                    0x4000004c9L
#define SDB_CATALOGMGR__ALTERCOLLECTIONOLD                 0x4000004caL
#define SDB_CATALOGMGR_CREATECS                            0x4000004cbL
#define SDB_CATALOGMGR_CREATECL                            0x4000004ccL
#define SDB_CATALOGMGR_CMDSPLIT                            0x4000004cdL
#define SDB_CATALOGMGR__CHECKCSOBJ                         0x4000004ceL
#define SDB_CATALOGMGR__CHECKANDBUILDCATARECORD            0x4000004cfL
#define SDB_CATALOGMGR__ASSIGNGROUP                        0x4000004d0L
#define SDB_CATALOGMGR__CHECKGROUPINDOMAIN                 0x4000004d1L
#define SDB_CATALOGMGR__CREATECS                           0x4000004d2L
#define SDB_CATALOGMGR_CREATECOLLECTION                    0x4000004d3L
#define SDB_CATALOGMGR_BUILDCATALOGRECORD                  0x4000004d4L
#define SDB_CATALOGMGR_BUILDINITBOUND                      0x4000004d5L
#define SDB_CATALOGMGR_PROCESSMSG                          0x4000004d6L
#define SDB_CATALOGMGR_PROCESSCOMMANDMSG                   0x4000004d7L
#define SDB_CATALOGMGR__BUILDHASHBOUND                     0x4000004d8L
#define SDB_CATALOGMGR_CMDLINKCOLLECTION                   0x4000004d9L
#define SDB_CATALOGMGR_CMDUNLINKCOLLECTION                 0x4000004daL
#define SDB_CATALOGMGR_CREATEDOMAIN                        0x4000004dbL
#define SDB_CATALOGMGR_DROPDOMAIN                          0x4000004dcL
#define SDB_CATALOGMGR_ALTERDOMAIN                         0x4000004ddL
#define SDB_CATALOGMGR__BUILDALTERGROUPS                   0x4000004deL
#define SDB_CATALOGMGR__CHOOSEFGROUPOFCL                   0x4000004dfL
#define SDB_CATALOGMGR_AUTOHASHSPLIT                       0x4000004e0L
#define SDB_CATALOGMGR__COMBINEOPTIONS                     0x4000004e1L
#define SDB_CATALOGMGR__BUILDALTEROBJWITHMETAANDOBJ        0x4000004e2L
#define SDB_CATALOGMGR__GETGROUPSOFCOLLECTIONS             0x4000004e3L
#endif
