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
#ifndef ossTRACE_H__
#define ossTRACE_H__
// Component: oss
#define SDB_OSSOPEN                                        0x100000000cdL
#define SDB_OSSCLOSE                                       0x100000000ceL
#define SDB_OSSMKDIR                                       0x100000000cfL
#define SDB_OSSDELETE                                      0x100000000d0L
#define SDB_OSSFILECOPY                                    0x100000000d1L
#define SDB_OSSACCESS                                      0x100000000d2L
#define SDB_OSSREAD                                        0x100000000d3L
#define SDB_OSSWRITE                                       0x100000000d4L
#define SDB_OSSSEEK                                        0x100000000d5L
#define SDB_OSSSEEKANDREAD                                 0x100000000d6L
#define SDB_OSSSEEKANDWRITE                                0x100000000d7L
#define SDB_OSSFSYNC                                       0x100000000d8L
#define SDB_OSSGETPATHTYPE                                 0x100000000d9L
#define SDB_OSSGETFSBYNM                                   0x100000000daL
#define SDB_OSSGETFILESIZE                                 0x100000000dbL
#define SDB_OSSTRUNCATEFILE                                0x100000000dcL
#define SDB_OSSEXTFILE                                     0x100000000ddL
#define SDB_OSSGETREALPATH                                 0x100000000deL
#define SDB_OSSGETFSTYPE                                   0x100000000dfL
#define SDB_OSSRENMPATH                                    0x100000000e0L
#define SDB_OSSLOCKFILE                                    0x100000000e1L
#define SDB_OSSMODULEHANDLE_INIT                           0x100000000e2L
#define SDB_OSSMODULEHANDLE_UNLOAD                         0x100000000e3L
#define SDB_OSSMODULEHANDLE_RESOLVEADDRESS                 0x100000000e4L
#define SDB__OSSMEMALLOC                                   0x100000000e5L
#define SDB__OSSMEMREALLOC                                 0x100000000e6L
#define SDB__OSSMEMFREE                                    0x100000000e7L
#define SDB__OSSMMF_OPEN                                   0x100000000e8L
#define SDB__OSSMMF_CLOSE                                  0x100000000e9L
#define SDB__OSSMMF_SIZE                                   0x100000000eaL
#define SDB__OSSMMF_MAP                                    0x100000000ebL
#define SDB__OSSMMF_FLHALL                                 0x100000000ecL
#define SDB__OSSMMF_FLUSH                                  0x100000000edL
#define SDB__OSSMMF_FLUSHBLOCK                             0x100000000eeL
#define SDB__OSSMMF_UNLINK                                 0x100000000efL
#define SDB__OSSENUMNMPS                                   0x100000000f0L
#define SDB__OSSENUMNMPS2                                  0x100000000f1L
#define SDB_OSSCRTNMP                                      0x100000000f2L
#define SDB_OSSOPENNMP                                     0x100000000f3L
#define SDB_OSSCONNNMP                                     0x100000000f4L
#define SDB_OSSRENMP                                       0x100000000f5L
#define SDB_OSSWTNMP                                       0x100000000f6L
#define SDB_OSSDISCONNNMP                                  0x100000000f7L
#define SDB_OSSCLSNMP                                      0x100000000f8L
#define SDB_OSSNMP2FD                                      0x100000000f9L
#define SDB_OSSCRTNP                                       0x100000000faL
#define SDB_OSSOPENNP                                      0x100000000fbL
#define SDB_OSSRDNP                                        0x100000000fcL
#define SDB__OSSWTNP                                       0x100000000fdL
#define SDB_OSSDELNP                                       0x100000000feL
#define SDB_OSSNP2FD                                       0x100000000ffL
#define SDB_OSSCLNPBYNM                                    0x10000000100L
#define SDB__OSSSK__OSSSK                                  0x10000000101L
#define SDB__OSSSK__OSSSK2                                 0x10000000102L
#define SDB__OSSSK__OSSSK3                                 0x10000000103L
#define SDB_OSSSK_INITTSK                                  0x10000000104L
#define SDB_OSSSK_SETSKLI                                  0x10000000105L
#define SDB_OSSSK_KPAL                                     0x10000000106L
#define SDB_OSSSK_BIND_LSTN                                0x10000000107L
#define SDB_OSSSK_SEND                                     0x10000000108L
#define SDB_OSSSK_ISCONN                                   0x10000000109L
#define SDB_OSSSK_CONNECT                                  0x1000000010aL
#define SDB_OSSSK_CLOSE                                    0x1000000010bL
#define SDB_OSSSK_DISNAG                                   0x1000000010cL
#define SDB_OSSSK_SECURE                                   0x1000000010dL
#define SDB_OSSSK_DOSSLHANDSHAKE                           0x1000000010eL
#define SDB_OSSSK__GETADDR                                 0x1000000010fL
#define SDB_OSSSK_SETTMOUT                                 0x10000000110L
#define SDB__OSSSK__COMPLETE                               0x10000000111L
#define SDB_OSSPFOP_OPEN                                   0x10000000112L
#define SDB_OSSPFOP_READ                                   0x10000000113L
#define SDB_OSSPFOP_WRITE                                  0x10000000114L
#define SDB_OSSPFOP_FWRITE                                 0x10000000115L
#define SDB_OSSPFOP_GETSIZE                                0x10000000116L
#define SDB_GETEXECNM                                      0x10000000117L
#define SDB_OSSLCEXEC                                      0x10000000118L
#define SDB__OSSAIOMSGPROC__PROC                           0x10000000119L
#define SDB__OSSAIOMSGPROC__HNDWP                          0x1000000011aL
#define SDB__OSSAIOMSGPROC__HNDRPH                         0x1000000011bL
#define SDB__OSSAIOMSGPROC__RDPH                           0x1000000011cL
#define SDB__OSSAIOMSGPROC_CONNECT                         0x1000000011dL
#define SDB__TMPAIR_CHK_DLINE                              0x1000000011eL
#define SDB__TMPAIR_RUN                                    0x1000000011fL
#define SDB__OSSAIO__HNDAPT                                0x10000000120L
#define SDB__OSSAIO__ACCEPT                                0x10000000121L
#define SDB__OSSAIO_CONNECT                                0x10000000122L
#define SDB__OSSAIO_ADDTIMER                               0x10000000123L
#define SDB__OSSAIO_RMTIMER                                0x10000000124L
#define SDB_OSSISPROCRUNNING                               0x10000000125L
#define SDB_OSSWAITCHLD                                    0x10000000126L
#define SDB_OSSCRTLST                                      0x10000000127L
#define SDB_OSSEXEC2                                       0x10000000128L
#define SDB_OSSEXEC                                        0x10000000129L
#define SDB_OSSENBNMCHGS                                   0x1000000012aL
#define SDB_OSSRENMPROC                                    0x1000000012bL
#define SDB_OSSVERIFYPID                                   0x1000000012cL
#define SDB_OSSRSVPATH                                     0x1000000012dL
#define SDB_OSSWTINT                                       0x1000000012eL
#define SDB_OSSSTARTSERVICE                                0x1000000012fL
#define SDB_OSS_WFSTRS                                     0x10000000130L
#define SDB_OSS_STOPSERVICE                                0x10000000131L
#define SDB_OSSCRTPADUPHND                                 0x10000000132L
#define SDB_WIN_OSSEXEC                                    0x10000000133L
#define SDB_OSSGETEWD                                      0x10000000134L
#define SDB_OSSCMSTART_BLDARGS                             0x10000000135L
#define SDB_OSS_STARTPROCESS                               0x10000000136L
#define SDB_OSSRSTSYSSIG                                   0x10000000137L
#define SDB_OSSSIGHNDABT                                   0x10000000138L
#define SDB_OSSFUNCADDR2NM                                 0x10000000139L
#define SDB_OSSDUMPSYSTM                                   0x1000000013aL
#define SDB_OSSDUMPDBINFO                                  0x1000000013bL
#define SDB_OSSDUMPSYSINFO                                 0x1000000013cL
#define SDB_OSSMCHCODE                                     0x1000000013dL
#define SDB_OSSDUMPSIGINFO                                 0x1000000013eL
#define SDB_OSSWLKSTK                                      0x1000000013fL
#define SDB_OSSGETSYMBNFA                                  0x10000000140L
#define SDB_OSSDUMPREGSINFO                                0x10000000141L
#define SDB_OSSDUMPST                                      0x10000000142L
#define SDB_OSSDUMPREGSINFO2                               0x10000000143L
#define SDB_OSSDUMPST2                                     0x10000000144L
#define SDB_OSSDUMPREGSINFO3                               0x10000000145L
#define SDB_OSSDUMPST3                                     0x10000000146L
#define SDB_OSSSYMINIT                                     0x10000000147L
#define SDB_OSSWKSEX                                       0x10000000148L
#define SDB_OSSWS                                          0x10000000149L
#define SDB_OSSGETSYMBNFADDR                               0x1000000014aL
#define SDB__OSSEVN_WAIT                                   0x1000000014bL
#define SDB__OSSEN_SIGNAL                                  0x1000000014cL
#define SDB__OSSEN_SIGALL                                  0x1000000014dL
#define SDB__OSSVN_RESET                                   0x1000000014eL
#define SDB_OSSTS2STR                                      0x1000000014fL
#define SDB_OSSGETCPUUSG                                   0x10000000150L
#define SDB_OSSGETCPUUSG2                                  0x10000000151L
#define SDB_OSSSRAND                                       0x10000000152L
#define SDB_OSSRAND                                        0x10000000153L
#define SDB_OSSHEXDL                                       0x10000000154L
#define SDB_OSSHEXDUMPBUF                                  0x10000000155L
#define SDB_OSSGETMEMINFO                                  0x10000000156L
#define SDB_OSSGETDISKINFO                                 0x10000000157L
#define SDB_OSSGETCPUINFO                                  0x10000000158L
#define SDB_OSSGETPROCMEMINFO                              0x10000000159L
#define SDB__OSSRWM_LOCK_R                                 0x1000000015aL
#define SDB__OSSRWM_LOCK_W                                 0x1000000015bL
#define SDB__OSSRWM_RLS_R                                  0x1000000015cL
#define SDB__OSSRWM_RLS_W                                  0x1000000015dL
#define SDB_OSSNTHND                                       0x1000000015eL
#define SDB_OSSST                                          0x1000000015fL
#define SDB_OSSEDUCTHND                                    0x10000000160L
#define SDB_OSSEDUEXCFLT                                   0x10000000161L
#define SDB_OSSDMPSYSTM                                    0x10000000162L
#define SDB_OSSDMPDBINFO                                   0x10000000163L
#define SDB_OSSSTKTRA                                      0x10000000164L
#define SDB_OSSREGSIGHND                                   0x10000000165L
#endif
