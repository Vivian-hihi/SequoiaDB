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

// This Header File is automatically generated, you MUST NOT modify this file anyway!
// On the contrary, you can modify the xml file "sequoiadb/misc/autogen/optlist.xml" if necessary!

#ifndef PMDOPTIONS_HPP_
#define PMDOPTIONS_HPP_

#include "pmdOptions.h"
#define PMD_COMMANDS_OPTIONS \
        ( PMD_COMMANDS_STRING (PMD_OPTION_HELP, ",h"), "help" ) \
        ( PMD_OPTION_VERSION, "database version" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_CONFPATH, ",c"), boost::program_options::value<string>(), "configure file path" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_LOGPATH, ",l"), boost::program_options::value<string>(), "log file path" ) \
        ( PMD_OPTION_DIAGLOGPATH, boost::program_options::value<string>(), "diagnostic log file path" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_DBPATH, ",d"), boost::program_options::value<string>(), "database path" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_IDXPATH, ",i"), boost::program_options::value<string>(), "index path" ) \
        ( PMD_OPTION_BKUPPATH, boost::program_options::value<string>(), "backup path" ) \
        ( PMD_OPTION_MAXPOOL, boost::program_options::value<int>(), "max pooled agent,defalut:0" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_SVCNAME, ",p"), boost::program_options::value<string>(), "local service name or port" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_REPLNAME, ",r"), boost::program_options::value<string>(), "replication service name or port" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_SHARDNAME, ",a"), boost::program_options::value<string>(), "sharding service name or port" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_CATANAME, ",x"), boost::program_options::value<string>(), "catalog service name or port" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_RESTNAME, ",s"), boost::program_options::value<string>(), "REST service name or port" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_DIAGLEVEL, ",v"), boost::program_options::value<int>(), "diag level,default:3,value range:[0-5]" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_ROLE, ",o"), boost::program_options::value<string>(), "role of the node (data/coord/catalog/standalone)" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_CATALOG_ADDR, ",t"), boost::program_options::value<string>(), "catalog addr (hostname1:servicename1,hostname2:servicename2,...)" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_LOGFILESZ, ",f"), boost::program_options::value<int>(), "log file size(MB),default:64,value range:[64,2048]" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_LOGFILENUM, ",n"), boost::program_options::value<int>(), "number of log files,default:20, value range:[1,60000]" ) \
        ( PMD_COMMANDS_STRING (PMD_OPTION_TRANSACTIONON, ",e"), boost::program_options::value<string>(), "turn on transaction, default:FALSE" ) \
        ( PMD_OPTION_NUMPRELOAD, boost::program_options::value<int>(), "number of pre-loaders,default:0,value range:[0,100]" ) \
        ( PMD_OPTION_MAX_PREF_POOL, boost::program_options::value<int>(), "maximum number of prefetchers, default:200, value range:[0,1000]" ) \
        ( PMD_OPTION_MAX_SUB_QUERY, boost::program_options::value<int>(), "maximum number of query's sub-query, default:10, min value is 0, max value can't more than param 'maxprefpool'" ) \
        ( PMD_OPTION_MAX_REPL_SYNC, boost::program_options::value<int>(), "maximum number of repl-sync, default:2, value range:[0, 200], 0:disable concurrent repl-sync" ) \
        ( PMD_OPTION_LOGBUFFSIZE, boost::program_options::value<int>(), "the replica-log's memory page number,default is 1024, value range:[512,1024000], but total log memory size can't more than all log file size; a page size is 64KB" ) \
        ( PMD_OPTION_DMS_TMPBLKPATH, boost::program_options::value<string>(), "path of the tmp file" ) \
        ( PMD_OPTION_SORTBUF_SIZE, boost::program_options::value<int>(), "size of the sorting buf(MB)" ) \
        ( PMD_OPTION_HJ_BUFSZ, boost::program_options::value<int>(), "size of the hash join buf(MB)" ) \

#define PMD_HIDDEN_COMMANDS_OPTIONS \
        ( PMD_OPTION_REPL_BUCKET_SIZE, boost::program_options::value<int>(), "repl bucket size, must be the power of 2, default is 32, value range:[1,4096]" ) \
        ( PMD_OPTION_MEMDEBUG, boost::program_options::value<string>(), "enable memory debug, default:FALSE" ) \
        ( PMD_OPTION_MEMDEBUGSIZE, boost::program_options::value<int>(), "memory debug segment size,default:0, if not zero, the value range:[256,4194304]" ) \
        ( PMD_OPTION_CATALIST, boost::program_options::value<string>(), "catalog node list(json)" ) \
        ( PMD_OPTION_DPSLOCAL, boost::program_options::value<string>(), "log the operation from lcoal port, default:FALSE" ) \
        ( PMD_OPTION_TRACEON, boost::program_options::value<string>(), "turn on trace when starting, default:FALSE" ) \
        ( PMD_OPTION_TRACEBUFSZ, boost::program_options::value<int>(), "trace buffer for starting, default:268435456, value range:[524288,1073741824]" ) \
        ( PMD_OPTION_SHARINGBRK, boost::program_options::value<int>(), "when the time of heartbeat breaking become be this value, we think they lost contact(ms), default:5000, value range:[5000,300000] " ) \

#endif /* PMDOPTIONS_HPP_ */