// this file is only intened to be included by sdb.cpp and sdbbp.cpp
#ifndef SDBCOMMON_HPP__
#define SDBCOMMON_HPP__

#include "ossPrimitiveFileOp.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

namespace fs = boost::filesystem ;

#define CMD_HELP           "help"
#define CMD_QUIT           "quit"
#define CMD_QUIT1          "quit;"
#define CMD_CLEAR          "clear"
#define CMD_CLEAR1         "clear;"
#define CMD_CLEARHISTORY   "history-c"
#define CMD_CLEARHISTORY1  "history-c;"

#define PROC_PIPE_NAME_LEN 255
#if defined (_LINUX)
#define PROC_PATH          "/proc"
#define PROC_CMDLINE_PATH_FORMAT PROC_PATH"/%s/cmdline"
#define PROC_PATH_LEN_MAX 255
#define PROC_TEMP_BUF_SZ 256
#define PROC_STATUS_ZOMBIE 'Z'
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
//#define MAN_PATH_PRE "..\\doc\\manual\\"
#define LIST_TIMEOUT 10
#endif

#define PROG_PATH_LEN 255
#define PROG_NAME_LEN 255
//#define MAN_PATH_SUF "_en_v2.cli"


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
void setProgramName( const CHAR *name ) ;
const CHAR* getProgramName() ;
const CHAR* getProgramPath() ;

/*
INT32 getWaitPipeName ( const OSSPID & ppid , CHAR * buf , UINT32 bufSize )
{
   INT32          nWritten    = 0 ;
   INT32          rc          = SDB_OK ;
#if defined (_WINDOWS)
   const CHAR *   waitFormat  = "sdb-shell-wait-%u" ;
#else
   const CHAR *   waitFormat  = "/tmp/sdb-shell-wait-%u" ;
#endif

   SDB_ASSERT ( buf && bufSize > 0 , "invalid argument" ) ;

   nWritten = ossSnprintf ( buf , bufSize , waitFormat , ppid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < bufSize , SDB_INVALIDSIZE ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 getPipeNames( const OSSPID & ppid , CHAR * f2bName , UINT32 f2bSize ,
                    CHAR * b2fName , UINT32 b2fSize )
{
   INT32          rc          = SDB_OK ;
   INT32          nWritten    = 0 ;
#if defined (_WINDOWS)
   const CHAR *   f2bFormat   = "sdb-shell-f2b-%u" ;
   const CHAR *   b2fFormat   = "sdb-shell-b2f-%u" ;
#else
   const CHAR *   f2bFormat   = "sdb-shell-f2b-%u" ;
   const CHAR *   b2fFormat   = "sdb-shell-b2f-%u" ;
#endif

   SDB_ASSERT ( f2bName && b2fName && f2bSize > 0 && b2fSize > 0 ,
                "Invalid arguments" ) ;

   nWritten = ossSnprintf ( f2bName , f2bSize , f2bFormat , ppid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < f2bSize , SDB_INVALIDSIZE ) ;

   nWritten = ossSnprintf ( b2fName , b2fSize , b2fFormat , ppid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < b2fSize , SDB_INVALIDSIZE ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 getPipeNames2( const OSSPID & ppid , const OSSPID & pid ,
                     CHAR * f2bName , UINT32 f2bSize ,
                     CHAR * b2fName , UINT32 b2fSize )
{
   INT32          rc          = SDB_OK ;
   INT32          nWritten    = 0 ;
#if defined (_WINDOWS)
   const CHAR *   f2bFormat   = "sdb-shell-f2b-%u-%u" ;
   const CHAR *   b2fFormat   = "sdb-shell-b2f-%u-%u" ;
#else
   const CHAR *   f2bFormat   = "/tmp/sdb-shell-f2b-%u-%u" ;
   const CHAR *   b2fFormat   = "/tmp/sdb-shell-b2f-%u-%u" ;
#endif

   SDB_ASSERT ( f2bName && b2fName && f2bSize > 0 && b2fSize > 0 ,
                "Invalid arguments" ) ;

   nWritten = ossSnprintf ( f2bName , f2bSize , f2bFormat , ppid , pid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < f2bSize , SDB_INVALIDSIZE ) ;

   nWritten = ossSnprintf ( b2fName , b2fSize , b2fFormat , ppid , pid ) ;
   SH_VERIFY_COND ( nWritten >= 0 , SDB_SYS ) ;
   SH_VERIFY_COND ( (UINT32) nWritten < b2fSize , SDB_INVALIDSIZE ) ;

done :
   return rc ;
error :
   goto done ;
}

INT32 getPipeNames1( CHAR * bpf2bName , UINT32 bpf2bSize ,
                     CHAR * bpb2fName , UINT32 bpb2fSize ,
                     CHAR * f2bName , CHAR * b2fName )
{
   INT32           rc          = SDB_OK ;
   CHAR *          f2bp        = NULL ;
   CHAR *          b2fp        = NULL ;
   INT32           cnt         = 0 ;

   f2bp = bpf2bName ;
   b2fp = bpb2fName ;
#if defined (_WINDOWS)
   const CHAR *    path        = "\\\\.\\pipe\\" ;
#else
   const CHAR *    path        = "/tmp/" ;
   ossStrncpy ( bpf2bName , path , 5 ) ;
   f2bp += 5 ;
   ossStrncpy ( bpb2fName , path , 5 ) ;
   b2fp += 5 ;
#endif

   try
   {
      fs::path pipeDir( path ) ;
      fs::directory_iterator end_iter ;
      if ( fs::exists( pipeDir ) && fs::is_directory( pipeDir ) )
      {
         for ( fs::directory_iterator dir_iter( pipeDir );
               dir_iter != end_iter; ++dir_iter )
         {
            const std::string fileName =
                              dir_iter->path().filename().string() ;
            const CHAR *pFileName = fileName.c_str() ;
            if ( ossStrncmp ( pFileName , b2fName ,
                              ossStrlen ( b2fName ) ) == 0 )
            {
               ossStrncpy ( b2fp , pFileName , bpb2fSize-5 ) ;
               cnt++ ;
            }
            else if ( ossStrncmp ( pFileName , f2bName ,
                      ossStrlen ( f2bName ) ) == 0 )
            {
               ossStrncpy ( f2bp , pFileName , bpf2bSize-5 ) ;
               cnt++ ;
            }
            if ( cnt >= 2 )
               goto done ;
         }
         rc = SDB_FNE ;
         goto error ;
      }
      else
      {
         PD_RC_CHECK ( SDB_INVALIDARG, PDERROR,
                       "Given path %s is not a directory or not exist",
                       path ) ;
      }
   }
   catch ( std::exception &e )
   {
      PD_RC_CHECK ( SDB_SYS, PDERROR,
                    "Failed to iterate pipefile path %s: %s",
                    path, e.what() ) ;
   }

done :
   return rc ;
error :
   goto done ;
}
*/
#endif //SDBCOMMON_HPP__
