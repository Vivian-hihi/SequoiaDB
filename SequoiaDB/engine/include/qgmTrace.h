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
#ifndef qgmTRACE_H__
#define qgmTRACE_H__
// Component: qgm
#define SDB__QGMBUILDER_BUILDORDERBY                       0x800000000000623L
#define SDB__QGMBUILDER_BUILD1                             0x800000000000624L
#define SDB__QGMBUILDER_BUILD2                             0x800000000000625L
#define SDB__QGMBUILDER__BUILDPHYNODE                      0x800000000000626L
#define SDB__QGMBUILDER__ADDPHYCOMMAND                     0x800000000000627L
#define SDB__QGMBUILDER__ADDPHYAGGR                        0x800000000000628L
#define SDB__QGMBUILDER__CRTPHYSORT                        0x800000000000629L
#define SDB__QGMBUILDER__ADDPHYSCAN                        0x80000000000062aL
#define SDB__QGMBUILDER__ADDMTHMATHERSCAN                  0x80000000000062bL
#define SDB__QGMBUILDER__CRTPHYJOIN                        0x80000000000062cL
#define SDB__QGMBUILDER__CRTPHYFILTER                      0x80000000000062dL
#define SDB__QGMBUILDER__CRTMTHMATHERFILTER                0x80000000000062eL
#define SDB__QGMBUILDER__BUILD1                            0x80000000000062fL
#define SDB__QGMBUILDER__BUILDUPDATE                       0x800000000000630L
#define SDB__QGMBUILDER__ADDSET                            0x800000000000631L
#define SDB__QGMBUILDER__BUILDDELETE                       0x800000000000632L
#define SDB__QGMBUILDER__BUILDDROPCL                       0x800000000000633L
#define SDB__QGMBUILDER__BUILDDROPINX                      0x800000000000634L
#define SDB__QGMBUILDER__BUILDCRTINX                       0x800000000000635L
#define SDB__QGMBUILDER__BUILDINXCOLUMNS                   0x800000000000636L
#define SDB__QGMBUILDER__BUILDCRTCL                        0x800000000000637L
#define SDB__QGMBUILDER__BUILDCRTCS                        0x800000000000638L
#define SDB__QGMBUILDER__BUILDDROPCS                       0x800000000000639L
#define SDB__QGMBUILDER__BUILDSELECT                       0x80000000000063aL
#define SDB__QGMBUILDER__BUILDINSERT                       0x80000000000063bL
#define SDB__QGMBUILDER__ADDSELECTOR                       0x80000000000063cL
#define SDB__QGMBUILDER__ADDFROM                           0x80000000000063dL
#define SDB__QGMBUILDER__BUILDJOIN                         0x80000000000063eL
#define SDB__QGMBUILDER__BUILDINCONDITION                  0x80000000000063fL
#define SDB__QGMBUILDER__BUILDCONDITION                    0x800000000000640L
#define SDB__QGMBUILDER__ADDSPLITBY                        0x800000000000641L
#define SDB__QGMBUILDER__ADDGROUPBY                        0x800000000000642L
#define SDB__QGMBUILDER__ADDORDERBY                        0x800000000000643L
#define SDB__QGBUILDER__ADDLIMIT                           0x800000000000644L
#define SDB__QGMBUILDER__ADDSKIP                           0x800000000000645L
#define SDB__QGMBUILDER__ADDCOLUMNS                        0x800000000000646L
#define SDB__QGMBUILDER__ADDVALUES                         0x800000000000647L
#define SDB__QGMBUILDER__ADDHINT                           0x800000000000648L
#define SDB__QGMBUILDER__ADDSELECTORFROMEXPR               0x800000000000649L
#define SDB__QGMBUILDER__BUILDEXPRTREE                     0x80000000000064aL
#define SDB__QGMCONDITIONNODEHELPER_MERGE                  0x80000000000064bL
#define SDB__QGMCONDITIONNODEHELPER_SEPARATE               0x80000000000064cL
#define SDB__QGMCONDITIONNODEHELPER_SEPARATE2              0x80000000000064dL
#define SDB__QGMCONDITIONNODEHELPER__CRTBSON               0x80000000000064eL
#define SDB__QGMCONDITIONNODEHELPER__TOBSON                0x80000000000064fL
#define SDB__QGMCONDITIONNODEHELPER__GETALLATTR            0x800000000000650L
#define SDB__QGMFETCHOUT_ELEMENT                           0x800000000000651L
#define SDB__QGMFETCHOUT_ELEMENTS                          0x800000000000652L
#define SDB__QGMEXTENDPLAN_EXTEND                          0x800000000000653L
#define SDB__QGMEXTENDPLAN_INSERTPLAN                      0x800000000000654L
#define SDB__QGMEXTENDSELECTPLAN__EXTEND                   0x800000000000655L
#define SDB__QGMHASHTBL_PUSH                               0x800000000000656L
#define SDB__QGMHASHTBL_FIND                               0x800000000000657L
#define SDB__QGMHASHTBL_GETMORE                            0x800000000000658L
#define SDB__QGMMATCHER_MATCH                              0x800000000000659L
#define SDB__QGMMATCHER__MATCH                             0x80000000000065aL
#define SDB__QGMOPTIAGGREGATION_INIT                       0x80000000000065bL
#define SDB__QGMOPTIAGGREGATION_DONE                       0x80000000000065cL
#define SDB__QGMOPTIAGGREGATION__UPDATE2UNIT               0x80000000000065dL
#define SDB__QGMOPTIAGGREGATION__ADDFIELDS                 0x80000000000065eL
#define SDB__QGMOPTIAGGREGATION__GETFIELDALIAS             0x80000000000065fL
#define SDB__QGMOPTIAGGREGATION__PUSHOPRUNIT               0x800000000000660L
#define SDB__QGMOPTIAGGREGATION_OURPUTSORT                 0x800000000000661L
#define SDB__QGMOPTIAGGREGATION_PARSE                      0x800000000000662L
#define SDB__QGMOPTIAGGREGATION_HASEXPR                    0x800000000000663L
#define SDB__QGMOPTINLJOIN__MAKECONDVAR                    0x800000000000664L
#define SDB__QGMOPTINLJOIN_MAKECONDITION                   0x800000000000665L
#define SDB__QGMOPTINLJOIN_INIT                            0x800000000000666L
#define SDB__QGMOPTINLJOIN__CRTJOINUNIT                    0x800000000000667L
#define SDB__QGMOPTINLJOIN_OUTPUTSTREAM                    0x800000000000668L
#define SDB__QGMOPTINLJOIN__PUSHOPRUNIT                    0x800000000000669L
#define SDB__QGMOPTINLJOIN__UPDATECHANGE                   0x80000000000066aL
#define SDB__QGMOPTINLJOIN_HANDLEHINTS                     0x80000000000066bL
#define SDB__QGMOPTINLJOIN__VALIDATE                       0x80000000000066cL
#define SDB__QGMOPTISELECT_INIT                            0x80000000000066dL
#define SDB__QGMOPTISELECT_DONE                            0x80000000000066eL
#define SDB__QGMOPTISELECT_OUTPUTSTREAM                    0x80000000000066fL
#define SDB__QGMOPTISELECT__PUSHOPRUNIT                    0x800000000000670L
#define SDB__QGMOPTISELECT__RMOPRUNIT                      0x800000000000671L
#define SDB__QGMOPTISELECT__EXTEND                         0x800000000000672L
#define SDB__QGMOPTISELECT__VALIDATEANDCRTPLAN             0x800000000000673L
#define SDB__QGMOPTISELECT__PARAMEXISTINSELECOTR           0x800000000000674L
#define SDB__QGMOPTISELECT_HASEXPR                         0x800000000000675L
#define SDB__QGMOPTISORT_INIT                              0x800000000000676L
#define SDB__QGMOPTISORT__PUSHOPRUNIT                      0x800000000000677L
#define SDB__QGMOPTISORT__RMOPRUNIT                        0x800000000000678L
#define SDB__QGMOPTISORT_APPEND                            0x800000000000679L
#define SDB__QGMOPSTREAM_FIND                              0x80000000000067aL
#define SDB__QGMOPRUNIT_ADDOPFIELD                         0x80000000000067bL
#define SDB__QGMOPTITREENODE_ADDCHILD                      0x80000000000067cL
#define SDB__QGMOPTITREENODE__ONPUSHOPRUNIT                0x80000000000067dL
#define SDB__QGMOPTITREENODE_PUSHOPRUNIT                   0x80000000000067eL
#define SDB__QGMOPTITREENODE_RMOPRUNIT                     0x80000000000067fL
#define SDB__QGMOPTITREENODE_UPCHANGE                      0x800000000000680L
#define SDB__QGMOPTITREENODE_OUTPUTSORT                    0x800000000000681L
#define SDB__QGMOPTITREENODE_EXTEND                        0x800000000000682L
#define SDB__QGMOPTTREE__PREPARE                           0x800000000000683L
#define SDB__QGMOPTTREE_INSERTBETWEEN                      0x800000000000684L
#define SDB__QGMPARAMTABLE_ADDCONST                        0x800000000000685L
#define SDB__QGMPARAMTABLE_ADDCONST2                       0x800000000000686L
#define SDB__QGMPARATABLE_ADDVAR                           0x800000000000687L
#define SDB__QGMPARAMTABLE_SETVAR                          0x800000000000688L
#define SDB__QGMPLAN_EXECUTE                               0x800000000000689L
#define SDB__QGMPLAN_FETCHNEXT                             0x80000000000068aL
#define SDB__QGMPLCOMMAND__EXEC                            0x80000000000068bL
#define SDB__QGMPLCOMMAND_EXECONCOORD                      0x80000000000068cL
#define SDB__QGMPLCOMMAND__EXECONDATA                      0x80000000000068dL
#define SDB__QGMPLCOMMAND__FETCHNEXT                       0x80000000000068eL
#define SDB__QGMPLDELETE__EXEC                             0x80000000000068fL
#define SDB__QGMPLFILTER__FETCHNEXT                        0x800000000000690L
#define SDB__QGMPLHASHJOIN_INIT                            0x800000000000691L
#define SDB__QGMPLHASHJOIN__EXEC                           0x800000000000692L
#define SDB__QGMPLHASHJOIN__FETCHNEXT                      0x800000000000693L
#define SDB__QGMPLHASHJOIN__BUILDHASNTBL                   0x800000000000694L
#define SDB__QGMPLINSERT__MERGEOBJ                         0x800000000000695L
#define SDB__QGMPLINSERT__NEXTRECORD                       0x800000000000696L
#define SDB__QGMPLINSERT__EXEC                             0x800000000000697L
#define SDB__QGMPLMTHMATCHERFILTER__FETCHNEXT              0x800000000000698L
#define SDB__QGMPLMTHMATCHERSCAN__EXEC                     0x800000000000699L
#define SDB__QGMPLNLJOIN__INIT                             0x80000000000069aL
#define SDB__QGMPLNLJOIN__EXEC                             0x80000000000069bL
#define SDB__QGMPLNLJOIN__FETCHNEXT                        0x80000000000069cL
#define SDB__QGMPLNLJOIN__MODIFYINNERCONDITION             0x80000000000069dL
#define SDB__QGMPLSCAN__EXEC                               0x80000000000069eL
#define SDB__QGMPLSCAN__EXECONDATA                         0x80000000000069fL
#define SDB__QGMPLSCAN__EXECONCOORD                        0x8000000000006a0L
#define SDB__QGMPLSCAN__FETCHNEXT                          0x8000000000006a1L
#define SDB__QGMPLSCAN__FETCH                              0x8000000000006a2L
#define SDB__QGMPLSORT__EXEC                               0x8000000000006a3L
#define SDB__QGMPLSORT__FETCHNEXT                          0x8000000000006a4L
#define SDB__QGMPLSPLITBY__FETCHNEXT                       0x8000000000006a5L
#define SDB__QGMPLUPDATE__EXEC                             0x8000000000006a6L
#define SDB__QGMPTRTABLE_GETFIELD                          0x8000000000006a7L
#define SDB__QGMPTRTABLE_GETOWNFIELD                       0x8000000000006a8L
#define SDB__QGMPTRTABLE_GETATTR                           0x8000000000006a9L
#define SDB__QGMSELECTOR_SELECT                            0x8000000000006aaL
#define SDB__QGMSELECTOR_SELECT2                           0x8000000000006abL
#define SDB__QGMSELECTOR__CREATEVALUEWITHEXPR              0x8000000000006acL
#define SDB__QGMSELECTOREXPR_GETVALUE                      0x8000000000006adL
#define SDB__QGMSELECTOREXPRNODE_GETVALUE                  0x8000000000006aeL
#define SDB__QGMSELECTOREXPRNODE__CALCVALUE                0x8000000000006afL
#define SDB__QGMUTILFIRSTDOT                               0x8000000000006b0L
#define SDB__QGMFINDFIELDFROMFUNC                          0x8000000000006b1L
#define SDB__QGMISFROMONE                                  0x8000000000006b2L
#define SDB__QGMISSAMEFROM                                 0x8000000000006b3L
#define SDB__QGMMERGE                                      0x8000000000006b4L
#define SDB__QGMREPLACEFIELDRELE                           0x8000000000006b5L
#define SDB__QGMREPLACEATTRRELE                            0x8000000000006b6L
#define SDB__QGMREPLACEATTRRELE2                           0x8000000000006b7L
#define SDB__QGMREPLACEATTRRELE3                           0x8000000000006b8L
#define SDB__QDMDOWNFIELDSBYFIELDALIAS                     0x8000000000006b9L
#define SDB__QGMDOWNATTRSBYFIELDALIAS                      0x8000000000006baL
#define SDB__QGMDOWNATTRSBYFIELDALIAS2                     0x8000000000006bbL
#define SDB__QGMDOWNAATTRBYFIELDALIAS                      0x8000000000006bcL
#define SDB__QGMDOWNAGGRSBYFIELDALIAS                      0x8000000000006bdL
#define SDB__QGMUPFIELDSBYFIELDALIAS                       0x8000000000006beL
#define SDB__QGMUPATTRSBYFIELDALIAS                        0x8000000000006bfL
#define SDB__QGMUPATTRSBYFIELDALIAS2                       0x8000000000006c0L
#define SDB__QGMUPAATTRBYFIELDALIAS                        0x8000000000006c1L
#define SDB__QGMUPATTRSBYFIELDALIAS3                       0x8000000000006c2L
#endif
