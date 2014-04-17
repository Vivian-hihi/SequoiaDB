// this file is only intened to be included by sdb.cpp and sdbbp.cpp
#ifndef UTILPIPE_HPP__
#define UTILPIPE_HPP__

#include "core.h"
/*
#include "ossPrimitiveFileOp.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

namespace fs = boost::filesystem ;


#if defined (_LINUX)
#define PROC_PATH          "/proc"
#define PROC_CMDLINE_PATH_FORMAT PROC_PATH"/%s/cmdline"
#define ENGINE_NAME "sequoiadb"
#define MODIFIED_ENGINE_NAME ENGINE_NAME "("
//#define ENGINE_NAME_PATTERN "sequoiadb(%s)"
#define ENGINE_NAME_PATTERN1 "sequoiadb("
#define ENGINE_NAME_PATTERN2 ")"
#define ENGINE_NAME_PATTERN ENGINE_NAME_PATTERN1 "%s" ENGINE_NAME_PATTERN2
//#define MAN_PATH_PRE "../doc/manual/"
#elif defined (_WINDOWS)
#define ENGINE_NAME "sequoiadb.exe"
#define ENGINE_NPIPE_PATTERN1 "sequoiadb"
#define ENGINE_NPIPE_PATTERN2 "engine"
#define ENGINE_NPIPE_PATTERN_SEP "_"
#define ENGINE_NPIPE_PATTERN ENGINE_NPIPE_PATTERN1 ENGINE_NPIPE_PATTERN_SEP \
                             ENGINE_NPIPE_PATTERN2 ENGINE_NPIPE_PATTERN_SEP \
                             "%s"
#define ENGINE_NPIPE_MSG_PID "$pid"

#define LIST_TIMEOUT 10
#endif
*/

#define SH_VERIFY_RC                                             \
   if ( rc != SDB_OK ) {                                          \
      PD_LOG ( PDERROR , "%s, rc = %d" , getErrDesp(rc) , rc ) ;  \
      goto error ;                                                \
   }

#define SH_VERIFY_COND(cond, ret)  \
   if (!(cond)) {                   \
      rc=ret;                       \
      SH_VERIFY_RC                 \
   }

INT32 getWaitPipeName ( const OSSPID & ppid , CHAR * buf , UINT32 bufSize ) ;
INT32 getPipeNames( const OSSPID & ppid , CHAR * f2bName , UINT32 f2bSize ,
                    CHAR * b2fName , UINT32 b2fSize ) ;
INT32 getPipeNames2( const OSSPID & ppid , const OSSPID & pid ,
                     CHAR * f2bName , UINT32 f2bSize ,
                     CHAR * b2fName , UINT32 b2fSize ) ;
INT32 getPipeNames1( CHAR * bpf2bName , UINT32 bpf2bSize ,
                     CHAR * bpb2fName , UINT32 bpb2fSize ,
                     CHAR * f2bName , CHAR * b2fName ) ;

#endif //UTILPIPE_HPP__
