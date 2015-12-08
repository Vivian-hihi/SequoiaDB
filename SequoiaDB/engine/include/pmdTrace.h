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
#ifndef pmdTRACE_H__
#define pmdTRACE_H__
// Component: pmd
#define SDB_CMMINTHREADENTY                                0x20000000000573L
#define SDB_CMSTOP_TERMPROC                                0x20000000000574L
#define SDB_CMSTOP_MAIN                                    0x20000000000575L
#define SDB_PMDLOCALAGENTENTPNT                            0x20000000000576L
#define SDB__PMDTMHD                                       0x20000000000577L
#define SDB__PMDTMHD_DES                                   0x20000000000578L
#define SDB__PMDTMHD_HDTMOUT                               0x20000000000579L
#define SDB__PMDMSGHND                                     0x2000000000057aL
#define SDB__PMDMSGHND_DESC                                0x2000000000057bL
#define SDB__PMDMSGHND_CPMSG                               0x2000000000057cL
#define SDB__PMDMSGHND_HNDMSG                              0x2000000000057dL
#define SDB__PMDMSGHND_HNDCLOSE                            0x2000000000057eL
#define SDB__PMDMSGHND_ONSTOP                              0x2000000000057fL
#define SDB__PMDMSGHND_HNDSNMSG                            0x20000000000580L
#define SDB__PMDMSGHND_HNDMAINMSG                          0x20000000000581L
#define SDB__PMDMSGHND_POSTMAINMSG                         0x20000000000582L
#define SDB_PMDASYNCNETEP                                  0x20000000000583L
#define SDB__PMDSN                                         0x20000000000584L
#define SDB__PMDSN_DESC                                    0x20000000000585L
#define SDB__PMDSN_ATHIN                                   0x20000000000586L
#define SDB__PMDSN_ATHOUT                                  0x20000000000587L
#define SDB__PMDSN_CLEAR                                   0x20000000000588L
#define SDB__PMDSN__MKNAME                                 0x20000000000589L
#define SDB__PMDSN__LOCK                                   0x2000000000058aL
#define SDB__PMDSN__UNLOCK                                 0x2000000000058bL
#define SDB__PMDSN_WTATH                                   0x2000000000058cL
#define SDB__PMDSN_WTDTH                                   0x2000000000058dL
#define SDB__PMDSN_CPMSG                                   0x2000000000058eL
#define SDB__PMDSN_FRNBUF                                  0x2000000000058fL
#define SDB__PMDSN_POPBUF                                  0x20000000000590L
#define SDB__PMDSN_PSHBUF                                  0x20000000000591L
#define PMD_SESSMGR                                        0x20000000000592L
#define PMD_SESSMGR_DESC                                   0x20000000000593L
#define PMD_SESSMGR_INIT                                   0x20000000000594L
#define PMD_SESSMGR_FINI                                   0x20000000000595L
#define PMD_SESSMGR_FORCENTY                               0x20000000000596L
#define PMD_SESSMGR_ONTIMER                                0x20000000000597L
#define PMD_SESSMGR_PUSHMSG                                0x20000000000598L
#define PMD_SESSMGR_GETSESSION                             0x20000000000599L
#define CLS_PMDSMGR_ATCHMETA                               0x2000000000059aL
#define PMD_SESSMGR_STARTEDU                               0x2000000000059bL
#define PMD_SESSMGR_RLSSS                                  0x2000000000059cL
#define PMD_SESSMGR_RLSSS_I                                0x2000000000059dL
#define PMD_SESSMGR_REPLY                                  0x2000000000059eL
#define PMD_SESSMGR_HDLSNCLOSE                             0x2000000000059fL
#define PMD_SESSMGR_HDLSTOP                                0x200000000005a0L
#define PMD_SESSMGR_HDLSNTM                                0x200000000005a1L
#define PMD_SESSMGR_CHKSNMETA                              0x200000000005a2L
#define PMD_SESSMGR_CHKFORCESN                             0x200000000005a3L
#define PMD_SESSMGR_CHKSN                                  0x200000000005a4L
#define SDB_PMDSYNCSESSIONAGENTEP                          0x200000000005a5L
#define SDB_PMDBGJOBENTPNT                                 0x200000000005a6L
#define SDB_PMDCBMGREP                                     0x200000000005a7L
#define SDB_PMDCOORDNETWKENTPNT                            0x200000000005a8L
#define SDB_REGEDUNAME                                     0x200000000005a9L
#define SDB__PMDEDUCB_DISCONNECT                           0x200000000005aaL
#define SDB__PMDEDUCB_FORCE                                0x200000000005abL
#define SDB__PMDEDUCB_ISINT                                0x200000000005acL
#define SDB__PMDEDUCB_PRINTINFO                            0x200000000005adL
#define SDB__PMDEDUCB_GETINFO                              0x200000000005aeL
#define SDB__PMDEDUCB_RESETINFO                            0x200000000005afL
#define SDB__PMDEDUCB_CONTXTPEEK                           0x200000000005b0L
#define SDB___PMDEDUCB_DUMPINFO                            0x200000000005b1L
#define SDB___PMDEDUCB_DUMPINFO2                           0x200000000005b2L
#define SDB__PMDEDUCB_GETTRANSLOCK                         0x200000000005b3L
#define SDB__PMDEDUCB_ADDLOCKINFO                          0x200000000005b4L
#define SDB__PMDEDUCB_DELLOCKINFO                          0x200000000005b5L
#define SDB__PMDEDUCB_CREATETRANSACTION                    0x200000000005b6L
#define SDB__PMDEDUCB_DELTRANSACTION                       0x200000000005b7L
#define SDB__PMDEDUCB_ADDTRANSNODE                         0x200000000005b8L
#define SDB__PMDEDUCB_GETTRANSNODEROUTEID                  0x200000000005b9L
#define SDB__PMDEDUCB_ISTRANSNODE                          0x200000000005baL
#define SDB_PMDEDUENTPNT                                   0x200000000005bbL
#define SDB_PMDRECV                                        0x200000000005bcL
#define SDB_PMDSEND                                        0x200000000005bdL
#define SDB__PMDEDUMGR_DELIOSVC                            0x200000000005beL
#define SDB__PMDEDUMGR_DUMPINFO                            0x200000000005bfL
#define SDB__PMDEDUMGR_DUMPINFO2                           0x200000000005c0L
#define SDB__PMDEDUMGR_DESTROYALL                          0x200000000005c1L
#define SDB__PMDEDUMGR_FORCEUSREDU                         0x200000000005c2L
#define SDB__PMDEDUMGR__FORCEIOSVC                         0x200000000005c3L
#define SDB__PMDEDUMGR__FORCEEDUS                          0x200000000005c4L
#define SDB__PMDEDUMGR__GETEDUCNT                          0x200000000005c5L
#define SDB__PMDEDUMGR_PSTEDUPST                           0x200000000005c6L
#define SDB__PMDEDUMGR_RTNEDU                              0x200000000005c7L
#define SDB__PMDEDUMGR_STARTEDU                            0x200000000005c8L
#define SDB__PMDEDUMGR_CRTNEWEDU                           0x200000000005c9L
#define SDB__PMDEDUMGR_DSTEDU                              0x200000000005caL
#define SDB__PMDEDUMGR_WAITEDU                             0x200000000005cbL
#define SDB__PMDEDUMGR_WAITEDU2                            0x200000000005ccL
#define SDB__PMDEDUMGR_DEATVEDU                            0x200000000005cdL
#define SDB__PMDEDUMGR_ATVEDU                              0x200000000005ceL
#define SDB__PMDEDUMGR_ATVEDU2                             0x200000000005cfL
#define SDB__PMDEDUMGR_WAITUTIL                            0x200000000005d0L
#define SDB__PMDEDUMGR_WAITUTIL2                           0x200000000005d1L
#define SDB__PMDEDUMGR_GETEDUTRDID                         0x200000000005d2L
#define SDB_PMDSIGHND                                      0x200000000005d3L
#define SDB_PMDEDUUSERTRAPHNDL                             0x200000000005d4L
#define SDB_PMDCTRLHND                                     0x200000000005d5L
#define SDB_PMDLOADWORKER                                  0x200000000005d6L
#define SDB_PMDLOGGWENTPNT                                 0x200000000005d7L
#define SDB_PMDRESVARGS                                    0x200000000005d8L
#define SDB_PMDMSTTHRDMAIN                                 0x200000000005d9L
#define SDB_PMDMAIN                                        0x200000000005daL
#define SDB__PMDMEMPOL_ALLOC                               0x200000000005dbL
#define SDB__PMDMEMPOL_RELEASE                             0x200000000005dcL
#define SDB__PMDMEMPOL_REALLOC                             0x200000000005ddL
#define SDB__PMDOPTMGR_INIT                                0x200000000005deL
#define SDB__PMDOPTMGR__MKDIR                              0x200000000005dfL
#define SDB__PMDOPTMGR_REFLUSH2FILE                        0x200000000005e0L
#define SDB_PMDPRELOADERENENTPNT                           0x200000000005e1L
#define SDB_PMDRESTSN_PROMSG                               0x200000000005e2L
#define SDB_PMDLOCALSN_PROMSG                              0x200000000005e3L
#define SDB__PMDSTARTUP_INIT                               0x200000000005e4L
#define SDB__PMDSTARTUP_FINAL                              0x200000000005e5L
#define SDB_PMDTCPLSTNENTPNT                               0x200000000005e6L
#define SDB_PMDPIPELSTNNPNTPNT                             0x200000000005e7L
#define SDB_PMDWINSTARTSVC                                 0x200000000005e8L
#define SDB_PMDWINSVC_STPSVC                               0x200000000005e9L
#define SDB_PMDWINSVCMAIN                                  0x200000000005eaL
#define SDB_PMDWINSVCREPSTATTOMGR                          0x200000000005ebL
#define SDB_PMDWINSVCCTRLHANDL                             0x200000000005ecL
#define SDB_READFILE                                       0x200000000005edL
#define SDB_PARSEARGUMENTS                                 0x200000000005eeL
#define SDB_ENTERBATCHMODE                                 0x200000000005efL
#define SDB_ENTERINTATVMODE                                0x200000000005f0L
#define SDB_FORMATARGS                                     0x200000000005f1L
#define SDB_CREATEDAEMONPROC                               0x200000000005f2L
#define SDB_ENTERFRONTENDMODE                              0x200000000005f3L
#define SDB_SDB_MAIN                                       0x200000000005f4L
#define SDB_READFROMPIPE                                   0x200000000005f5L
#define SDB_MONITOR_THREAD                                 0x200000000005f6L
#define SDB_CREATESHMONTHREAD                              0x200000000005f7L
#define SDB_ENTERDAEMONMODE                                0x200000000005f8L
#define SDB_SDBBP_MAIN                                     0x200000000005f9L
#define SDB_SDBINSPT_RESVARG                               0x200000000005faL
#define SDB_FLUSHOUTPUT                                    0x200000000005fbL
#define SDB_DUMPPRINTF                                     0x200000000005fcL
#define SDB_REALLOCBUFFER                                  0x200000000005fdL
#define SDB_GETEXTBUFFER                                   0x200000000005feL
#define SDB_INSPECTHEADER                                  0x200000000005ffL
#define SDB_DUMPHEADER                                     0x20000000000600L
#define SDB_INSPECTSME                                     0x20000000000601L
#define SDB_DUMPSME                                        0x20000000000602L
#define SDB_GETEXTENTHEAD                                  0x20000000000603L
#define SDB_GETEXTENT                                      0x20000000000604L
#define SDB_EXTENTSANITYCHK                                0x20000000000605L
#define SDB_LOADMB                                         0x20000000000606L
#define SDB_LOADEXTENT                                     0x20000000000607L
#define SDB_INSPOVFLWRECRDS                                0x20000000000608L
#define SDB_DUMPOVFWRECRDS                                 0x20000000000609L
#define SDB_INSPINXDEF                                     0x2000000000060aL
#define SDB_DUMPINXDEF                                     0x2000000000060bL
#define SDB_INSPINXEXTS                                    0x2000000000060cL
#define SDB_DUMPINXEXTS                                    0x2000000000060dL
#define SDB_INSPCOLL                                       0x2000000000060eL
#define SDB_DUMPCOLL                                       0x2000000000060fL
#define SDB_INSPCOLLS                                      0x20000000000610L
#define SDB_DUMPCOLLS                                      0x20000000000611L
#define SDB_INSPCOLLECTIONS                                0x20000000000612L
#define SDB_DUMPRAWPAGE                                    0x20000000000613L
#define SDB_DUMPCOLLECTIONS                                0x20000000000614L
#define SDB_ACTIONCSATTEMPT                                0x20000000000615L
#define SDB_DUMPPAGES                                      0x20000000000616L
#define SDB_INSPECTDB                                      0x20000000000617L
#define SDB_SDBINSPT_MAIN                                  0x20000000000618L
#define SDB_SDBLIST_RESVARG                                0x20000000000619L
#define SDB_SDBLIST_MAIN                                   0x2000000000061aL
#define SDB_SDBLOAD_RESOLVEARG                             0x2000000000061bL
#define SDB_SDBLOAD_LOADRECV                               0x2000000000061cL
#define SDB_SDBLOAD_CONNECTSDB                             0x2000000000061dL
#define SDB_SDBLOAD_MAIN                                   0x2000000000061eL
#define SDB_SDBSTART_RESVARG                               0x2000000000061fL
#define SDB_SDBSTART_MAIN                                  0x20000000000620L
#define SDB_SDBSTOP_RESVARG                                0x20000000000621L
#define SDB_SDBSTOP_MAIN                                   0x20000000000622L
#endif
