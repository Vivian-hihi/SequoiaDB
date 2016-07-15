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
#ifndef dpsTRACE_H__
#define dpsTRACE_H__
// Component: dps
#define SDB__DPSLGRECD_LOADROWBODY                         0x1000000841L
#define SDB__DPSLGRECD_LOADBODY                            0x1000000842L
#define SDB__DPSLGRECD_LOAD                                0x1000000843L
#define SDB__DPSLGRECD_FIND                                0x1000000844L
#define SDB__DPSLGRECD_PUSH                                0x1000000845L
#define SDB__DPSLGRECD_DUMP                                0x1000000846L
#define SDB__DPSLGWRAPP_RECDROW                            0x1000000847L
#define SDB__DPSLGWRAPP_PREPARE                            0x1000000848L
#define SDB__DPSRPCMGR_INIT                                0x1000000849L
#define SDB__DPSRPCMGR__RESTRORE                           0x100000084aL
#define SDB__DPSRPCMGR_PREPAGES                            0x100000084bL
#define SDB__DPSRPCMGR_WRITEDATA                           0x100000084cL
#define SDB__DPSRPCMGR_MERGE                               0x100000084dL
#define SDB__DPSRPCMGR_GETLSNWIN                           0x100000084eL
#define SDB__DPSRPCMGR__MVPAGES                            0x100000084fL
#define SDB__DPSRPCMGR_MOVE                                0x1000000850L
#define SDB__DPSRPCMGR__GETSTARTLSN                        0x1000000851L
#define SDB__DPSRPCMGR__SEARCH                             0x1000000852L
#define SDB__DPSRPCMGR__PARSE                              0x1000000853L
#define SDB__DPSRPCMGR_SEARCH                              0x1000000854L
#define SDB__DPSRPCMGR__ALLOCATE                           0x1000000855L
#define SDB__DPSRPCMGR__PSH2SNDQUEUE                       0x1000000856L
#define SDB__DPSRPCMGR__MRGLOGS                            0x1000000857L
#define SDB__DPSRPCMGR__MRGPAGE                            0x1000000858L
#define SDB__DPSRPCMGR_RUN                                 0x1000000859L
#define SDB__DPSRPCMGR__FLUSHALL                           0x100000085aL
#define SDB__DPSRPCMGR_TEARDOWN                            0x100000085bL
#define SDB__DPSRPCMGR__FLUSHPAGE                          0x100000085cL
#define SDB__DPSRPCMGR_COMMIT                              0x100000085dL
#define SDB__DPSLOGFILE_INIT                               0x100000085eL
#define SDB__DPSLOGFILE__RESTRORE                          0x100000085fL
#define SDB__DPSLOGFILE_RESET                              0x1000000860L
#define SDB__DPSLOGFILE__FLUSHHD                           0x1000000861L
#define SDB__DPSLOGFILE__RDHD                              0x1000000862L
#define SDB__DPSLOGFILE_WRITE                              0x1000000863L
#define SDB__DPSLOGFILE_READ                               0x1000000864L
#define SDB__DPSLOGFILE_SYNC                               0x1000000865L
#define SDB__DPSMGBLK_CLEAR                                0x1000000866L
#define SDB_DPSTRANSCB_SVTRANSINFO                         0x1000000867L
#define SDB_DPSTRANSCB_ADDTRANSCB                          0x1000000868L
#define SDB_DPSTRANSCB_DELTRANSCB                          0x1000000869L
#define SDB_DPSTRANSCB_SAVETRANSINFOFROMLOG                0x100000086aL
#define SDB_DPSTRANSCB_TERMALLTRANS                        0x100000086bL
#define SDB_DPSLOCKBUCKET_ACQUIRE                          0x100000086cL
#define SDB_DPSLOCKBUCKET_WAITLOCKX                        0x100000086dL
#define SDB_DPSLOCKBUCKET_UPGRADE                          0x100000086eL
#define SDB_DPSLOCKBUCKET_LOCKID                           0x100000086fL
#define SDB_DPSLOCKBUCKET_APPENDTORUN                      0x1000000870L
#define SDB_DPSLOCKBUCKET_APPENDTOWAIT                     0x1000000871L
#define SDB_DPSLOCKBUCKET_APPENDHEADTOWAIT                 0x1000000872L
#define SDB_DPSLOCKBUCKET_REMOVEFROMRUN                    0x1000000873L
#define SDB_DPSLOCKBUCKET_REMOVEFROMWAIT                   0x1000000874L
#define SDB_DPSLOCKBUCKET_WAITLOCK                         0x1000000875L
#define SDB_DPSLOCKBUCKET_WAKEUP                           0x1000000876L
#define SDB_DPSLOCKBUCKET_CHECKCOMPATIBLE                  0x1000000877L
#define SDB_DPSLOCKBUCKET_TEST                             0x1000000878L
#define SDB_DPSLOCKBUCKET_TRYACQUIRE                       0x1000000879L
#define SDB_DPSLOCKBUCKET_TRYACQUIREORAPPEND               0x100000087aL
#define SDB_DPSLOCKBUCKET_HASWAIT                          0x100000087bL
#define SDB__DPS_INSERT2RECORD                             0x100000087cL
#define SDB_DPS_INSERT2RECORD                              0x100000087dL
#define SDB__DPS_UPDATE2RECORD                             0x100000087eL
#define SDB__DPS_RECORD2UPDATE                             0x100000087fL
#define SDB__DPS_DELETE2RECORD                             0x1000000880L
#define SDB__DPS_RECORD2DELETE                             0x1000000881L
#define SDB__DPS_CSCRT2RECORD                              0x1000000882L
#define SDB__DPS_RECORD2CSCRT                              0x1000000883L
#define SDB__DPS_CSDEL2RECORD                              0x1000000884L
#define SDB__DPS_RECORD2CSDEL                              0x1000000885L
#define SDB__DPS_CLCRT2RECORD                              0x1000000886L
#define SDB__DPS_RECORD2CLCRT                              0x1000000887L
#define SDB__DPS_CLDEL2RECORD                              0x1000000888L
#define SDB__DPS_RECORD2CLDEL                              0x1000000889L
#define SDB__DPS_IXCRT2RECORD                              0x100000088aL
#define SDB__DPS_RECORD2IXCRT                              0x100000088bL
#define SDB__DPS_IXDEL2RECORD                              0x100000088cL
#define SDB__DPS_RECORD2IXDEL                              0x100000088dL
#define SDB__DPS_CLRENAME2RECORD                           0x100000088eL
#define SDB__DPS_RECORD2CLRENAME                           0x100000088fL
#define SDB__DPS_CLTRUNC2RECORD                            0x1000000890L
#define SDB__DPS_RECORD2CLTRUNC                            0x1000000891L
#define SDB__DPS_RECORD2TRANSCOMMIT                        0x1000000892L
#define SDB__DPS_TRANSCOMMIT2RECORD                        0x1000000893L
#define SDB__DPS_TRANSROLLBACK2RECORD                      0x1000000894L
#define SDB__DPS_INVALIDCATA2RECORD                        0x1000000895L
#define SDB__DPS_RECORD2INVALIDCATA                        0x1000000896L
#define SDB__DPS_LOBW2RECORD                               0x1000000897L
#define SDB__DPS_RECORD2LOBW                               0x1000000898L
#define SDB__DPS_LOBU2RECORD                               0x1000000899L
#define SDB__DPS_RECORD2LOBU                               0x100000089aL
#define SDB__DPS_LOBRM2RECORD                              0x100000089bL
#define SDB__DPS_RECORD2LOBRM                              0x100000089cL
#define SDB__DPS_LOBTRUNCATE2RECORD                        0x100000089dL
#define SDB__DPS_RECORD2LOBTRUNCATE                        0x100000089eL
#define SDB__DPSMSGBLK_EXTEND                              0x100000089fL
#define SDB__DPSLGPAGE                                     0x10000008a0L
#define SDB__DPSLGPAGE2                                    0x10000008a1L
#define SDB_DPSTRANSLOCK_ACQUIREX                          0x10000008a2L
#define SDB_DPSTRANSLOCK_ACQUIRES                          0x10000008a3L
#define SDB_DPSTRANSLOCK_ACQUIREIX                         0x10000008a4L
#define SDB_DPSTRANSLOCK_ACQUIREIS                         0x10000008a5L
#define SDB_DPSTRANSLOCK_RELEASE                           0x10000008a6L
#define SDB_DPSTRANSLOCK_RELEASEALL                        0x10000008a7L
#define SDB_DPSTRANSLOCK_UPGRADE                           0x10000008a8L
#define SDB_DPSTRANSLOCK_UPGRADECHECK                      0x10000008a9L
#define SDB_DPSTRANSLOCK_GETBUCKET                         0x10000008aaL
#define SDB_DPSTRANSLOCK_TESTS                             0x10000008abL
#define SDB_DPSTRANSLOCK_TESTUPGRADE                       0x10000008acL
#define SDB_DPSTRANSLOCK_TESTIS                            0x10000008adL
#define SDB_DPSTRANSLOCK_TESTX                             0x10000008aeL
#define SDB_DPSTRANSLOCK_TESTIX                            0x10000008afL
#define SDB_DPSTRANSLOCK_TRYX                              0x10000008b0L
#define SDB_DPSTRANSLOCK_TRYS                              0x10000008b1L
#define SDB_DPSTRANSLOCK_TRYIX                             0x10000008b2L
#define SDB_DPSTRANSLOCK_TRYIS                             0x10000008b3L
#define SDB_DPSTRANSLOCK_TRYUPGRADE                        0x10000008b4L
#define SDB_DPSTRANSLOCK_TRYUPGRADEORAPPEND                0x10000008b5L
#define SDB_DPSTRANSLOCK_TRYORAPPENDX                      0x10000008b6L
#define SDB_DPSTRANSLOCK_WAIT                              0x10000008b7L
#define SDB_DPSTRANSLOCK_HASWAIT                           0x10000008b8L
#define SDB__DPSLGFILEMGR_INIT                             0x10000008b9L
#define SB__DPSLGFILEMGR__ANLYS                            0x10000008baL
#define SDB__DPSLGFILEMGR_FLUSH                            0x10000008bbL
#define SDB__DPSLGFILEMGR_LOAD                             0x10000008bcL
#define SDB__DPSLGFILEMGR_MOVE                             0x10000008bdL
#define SDB__DPSLGFILEMGR_SYNC                             0x10000008beL
#endif
