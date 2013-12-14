#include "spt.hpp"
#include <vector>
#include <string>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include "sdbOptionMgr.hpp"
#include "ossUtil.h"
#include "ossProc.hpp"
#include "ossNPipe.hpp"
#include "ossMem.h"
#include "ossMem.hpp"
#include "utilLinenoiseWrapper.hpp"
#include "oss.h"
#include "ossPath.h"
#include "pd.hpp"
#include "ossPrimitiveFileOp.hpp"
#include "ossTypes.h"
#include "ossVer.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

using std::ostream ;
using std::vector ;
using std::string ;
using std::bad_alloc ;
using namespace engine ;

namespace po = boost::program_options ;
po::options_description display ( "Command options" ) ;
po::positional_options_description destd ;
po::variables_map vm ;

#include "sdbcommon.hpp"

#if !defined (SDB_SHELL)
#error "sdbbp should always have SDB_SHELL defined"
#endif

extern INT32 gShellReturnCode ;

enum RunMode
{
   INTERACTIVE_MODE,
   BATCH_MODE,
   FRONTEND_MODE,
} ;

struct ArgInfo
{
   RunMode mode ;
   const CHAR * program ; // the program name
   const CHAR * filename ; // available in batch mode
   const CHAR * cmd ; // available in front-end mode
   const CHAR * variable ; // variable
} ;

PD_TRACE_DECLARE_FUNCTION ( SDB_READFILE, "readFile" )
INT32 readFile ( const CHAR * name , CHAR ** buf , UINT32 * size )
{
   PD_TRACE_ENTRY ( SDB_READFILE );
   ossPrimitiveFileOp op ;
   ossPrimitiveFileOp::offsetType offset ;
   INT32 rc = SDB_OK ;

   SDB_ASSERT ( name && buf, "Invalid arguments" ) ;

   rc = op.Open ( name , OSS_PRIMITIVE_FILE_OP_READ_WRITE ) ;
   if ( rc != SDB_OK )
   {
      ossPrintf ( "Can't open file: %s"OSS_NEWLINE, name ) ;
      goto error ;
   }

   rc = op.getSize ( &offset ) ;
   if ( rc != SDB_OK )
   {
      goto error ;
   }

   *buf = (CHAR *) SDB_OSS_MALLOC ( offset.offset ) ;
   if ( ! *buf )
   {
      rc = SDB_OOM ;
      PD_LOG ( PDERROR , "fail to alloc memory" ) ;
      goto error ;
   }

   rc = op.Read ( offset.offset , *buf , NULL ) ;
   if ( rc != SDB_OK )
   {
      goto error ;
   }

   if ( size ) *size = offset.offset ;

done :
   op.Close() ;
   PD_TRACE_EXITRC ( SDB_READFILE, rc );
   return rc ;
error :
   goto done ;
}

void printUsage()
{
   ossPrintf ( "Usage:\tsdb (Interactive mode)"OSS_NEWLINE ) ;
   ossPrintf ( "\t\tsdb -f <FILE> (Batch mode)"OSS_NEWLINE ) ;
   ossPrintf ( "\t\tsdb <CMD> (Deamon mode)"OSS_NEWLINE ) ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_PARSEARGUMENTS, "parseArguments" )
INT32 parseArguments ( int argc , CHAR ** argv , ArgInfo & argInfo )
{
   INT32 rc          = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_PARSEARGUMENTS );
   argInfo.mode      = INTERACTIVE_MODE ;
   argInfo.filename  = NULL ;
   argInfo.variable  = NULL ;
   argInfo.cmd       = NULL ;
   argInfo.program   = NULL ;
   string str        = "" ;

   SDB_POSITIONAL_OPTIONS_DESCRIPTION

   SDB_ADD_PARAM_OPTIONS_BEGIN ( display )
      SDB_COMMANDS_OPTIONS
   SDB_ADD_PARAM_OPTIONS_END

   try
   {
      po::store ( po::command_line_parser ( argc , argv ).options(
                  display ).positional ( destd ).run ()  , vm ) ;
      po::notify ( vm );
   }
   catch ( po::unknown_option &e )
   {
      std::cerr << "Unknown argument: "
                << e.get_option_name() << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      std::cerr << "Invalid argument: "
                << e.get_option_name() << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::error &e )
   {
      std::cerr << e.what() << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( vm.count( "help" ) )
   {
      std::cout << display << std::endl ;
      rc = SDB_SDB_HELP_ONLY ;
      goto done ;
   }
   if ( vm.count( "version" ) )
   {
      INT32 version, subVersion, release ;
      const CHAR *pBuild = NULL ;
      ossGetVersion ( &version, &subVersion, &release, &pBuild ) ;
      std::cout << "SequoiaDB shell version: " << version << "."
      << subVersion << std::endl ;
      std::cout << "Release: " << release << std::endl ;
      std::cout << "Build: " << pBuild <<std::endl ;
      rc = SDB_SDB_VERSION_ONLY ;
      goto done ;
   }
   if ( vm.count( "eval" ) )
   {
      str =  vm["eval"].as<string>() ;
      argInfo.variable = str.c_str() ;
   }

   SDB_ASSERT ( argv , "invalid argument" ) ;
   SDB_ASSERT ( argc >= 1 , "argc must be >= 1" ) ;

   argInfo.program = (const CHAR *) argv[0] ;

   if ( 1 == argc )
   {
      // Empty. Normal interactive mode
   }
   else if ( vm.count( "file" ) )
   {
      // Batch mode
      argInfo.mode = BATCH_MODE ;
      str =  vm["file"].as<string>() ;
      argInfo.filename = str.c_str() ;
   }
   else if ( vm.count( "shell" ) )
   {
      // Front-end mode
      argInfo.mode = FRONTEND_MODE ;
      str =  vm["shell"].as<string>() ;
      argInfo.cmd = str.c_str() ;
   }
   else
   {
      rc = SDB_INVALIDARG ;
      printUsage() ;
      goto error ;
   }

done :
   PD_TRACE_EXITRC ( SDB_PARSEARGUMENTS, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_ENTERBATCHMODE, "enterBatchMode" )
INT32 enterBatchMode( Scope * scope , const CHAR * filename ,
                                      const CHAR * variable )
{
   INT32    rc       = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_ENTERBATCHMODE );
   CHAR *   code     = NULL ;
   CHAR *   temp     = NULL ;
   CHAR *   vcode    = NULL ;
   CHAR *   p        = NULL ;
   CHAR *   result   = NULL ;
   UINT32   evalLen  = 0 ;
   UINT32   tempLen  = 0 ;
   UINT32   len      = 0 ;

   SDB_ASSERT ( scope , "invalid argument" ) ;
   SDB_ASSERT ( filename && filename[0] != '\0' , "invalid arguement" ) ;

   // code is freed in done:
   rc = readFile( filename , &temp , &len ) ;
   if ( rc != SDB_OK )
   {
      ossPrintf ( "fail to read file: %s"OSS_NEWLINE , filename ) ;
      goto error ;
   }

   if ( len > 0 )
   {
      //skip BOM (notepad auto add flag for UTF-8)
      if ( temp[0] == 0xEF && temp[1] == 0xBB && temp[1] == 0xEF ) 
      {
         vcode = &temp[3];
         len -= 3;
      }
      else 
      {
         vcode = temp;
      }
   
      if ( variable && variable[0] != '\0' )
      {
         evalLen = ossStrlen ( variable ) ;
         tempLen = len ;

         len = evalLen + tempLen + 2 ;
         code = ( CHAR * ) SDB_OSS_MALLOC ( len ) ;
         p = code ;

         ossStrncpy ( p , variable , evalLen ) ;
         p += evalLen ;
         *p = '\n' ;
         p = p + 1 ;
         ossStrncpy ( p , vcode , tempLen ) ;
         p += tempLen;
         *p = '\0' ;

         rc = scope->evaluate ( code , len-1 , filename , 1 , &result ) ;
      }
      else
      {
         rc = scope->evaluate ( vcode , len , filename , 1 , &result ) ;
      }
      if ( SDB_OK == rc )
      {
         SDB_ASSERT ( result, "evaluation succeed, but result is null" ) ;
         if ( result[0] != '\0' )
            ossPrintf ( "%s"OSS_NEWLINE, result ) ;
      }
   }
   else
   {
      ossPrintf( "File %s is empty."OSS_NEWLINE , filename ) ;
   }

done :
   SAFE_OSS_FREE ( code ) ;
   SAFE_OSS_FREE ( temp ) ;
   SAFE_OSS_FREE ( result ) ;
   PD_TRACE_EXITRC ( SDB_ENTERBATCHMODE, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_ENTERINTATVMODE, "enterInteractiveMode" )
INT32 enterInteractiveMode ( Scope *scope )
{
   INT32    rc          = SDB_OK ;
   CHAR *   result      = NULL ;
   CHAR *   code        = NULL ;
   INT64 sec            = 0 ;
   INT64 microSec       = 0 ;
   INT64 tkTime         = 0 ;
   ossTimestamp tmBegin ;
   ossTimestamp tmEnd ;
   string history ;


   SDB_ASSERT ( scope , "invalid argument" ) ;
   PD_TRACE_ENTRY ( SDB_ENTERINTATVMODE );

   // initialize and load the history
   historyInit () ;
   linenoiseHistoryLoad( historyFile.c_str() ) ;
   g_lnBuilder.loadCmd( historyFile.c_str() ) ;
   ossPrintf ( "Welcome to SequoiaDB shell!"OSS_NEWLINE ) ;
   ossPrintf ( "help() for help, Ctrl+c or quit to exit"OSS_NEWLINE ) ;

   while ( TRUE )
   {
      // code is freed in loop_next: or at the end of this function
      if ( ! getNextCommand ( "> ", &code ) )
         break ;

      if ( !code || '\0' == code[0] )
         goto loop_next ;

      if ( ossStrcmp ( CMD_QUIT, code ) == 0 ||
           ossStrcmp ( CMD_QUIT1,
                       boost::algorithm::erase_all_copy( string(code), " " ).c_str() ) == 0 )
      {
         linenoiseHistorySave( historyFile.c_str() ) ;
         break ;
      }
      else if ( ossStrcmp( CMD_CLEAR, code ) == 0 ||
                ossStrcmp( CMD_CLEAR1,
                boost::algorithm::erase_all_copy( string(code), " " ).c_str() ) == 0 )
      {
         linenoiseClearScreen() ;
         goto loop_next ;
      }
      history = string( code ) ;
      if ( ossStrcmp ( CMD_CLEARHISTORY,
           boost::algorithm::erase_all_copy( history, " " ).c_str() ) == 0 ||
           ossStrcmp ( CMD_CLEARHISTORY1,
           boost::algorithm::erase_all_copy( history, " " ).c_str() ) == 0 )
      {
         historyClear() ;
         goto loop_next ;
      }


      rc = SDB_OK ;
      ossGetCurrentTime ( tmBegin ) ;
      // result is freed in loop_next:
      rc = scope->evaluate ( code , 0 , "(shell)" , 1 , &result ) ;
      if ( SDB_OK == rc )
      {
         SDB_ASSERT ( result , "evaluation succeed, but result is null" ) ;
         if ( result[0] != '\0' )
            ossPrintf ( "%s"OSS_NEWLINE , result ) ;
      }
      ossGetCurrentTime ( tmEnd ) ;
      // takes time
      tkTime = ( tmEnd.time * 1000000 + tmEnd.microtm ) -
               ( tmBegin.time * 1000000 + tmBegin.microtm ) ;
      sec = tkTime/1000000 ;
      microSec = tkTime%1000000 ;
      ossPrintf ( "Takes %lld.%llds."OSS_NEWLINE , sec, microSec ) ;

      loop_next :
         SAFE_OSS_FREE ( code ) ;
         SAFE_OSS_FREE ( result ) ;
   }

   SAFE_OSS_FREE ( code ) ;
   PD_TRACE_EXITRC ( SDB_ENTERINTATVMODE, rc );
   return rc ;
}

// Concatenate into a string delimited by \0 and ended with \0\0
// caller should free *args in the case of success
PD_TRACE_DECLARE_FUNCTION ( SDB_FORMATARGS, "formatArgs" )
INT32 formatArgs ( const CHAR * program ,
                   const OSSPID & ppid ,
                   CHAR ** args )
{
   SDB_ASSERT ( program && program[0] != '\0' , "invalid argument" ) ;
   SDB_ASSERT ( args , "invalid argument" ) ;
   PD_TRACE_ENTRY ( SDB_FORMATARGS );

   INT32 rc          = SDB_OK ;
   INT32 progLen     = ossStrlen ( program ) ;
   INT32 ppidLen     = 0 ;
   INT32 argSize     = 0 ;
   CHAR *p           = NULL ;
   CHAR buf[128] ;
   INT32 bufSize     = sizeof ( buf ) / sizeof ( CHAR ) ;

   ossSnprintf ( buf , bufSize , "%u" , ppid ) ;

   ppidLen = ossStrlen ( buf ) ;
   argSize = progLen + 1 + ppidLen + 2 ;

   // caller is responsible for freeing *args
   *args = (CHAR*) SDB_OSS_MALLOC ( argSize ) ;
   if ( ! args )
   {
      rc = SDB_OOM ;
      SH_VERIFY_RC
   }

   p = *args ;
   ossStrcpy ( p , program ) ;
   p += progLen + 1 ;

   ossStrcpy( p , buf ) ;
   p += ppidLen + 1 ;
   *p = '\0' ;

done :
   PD_TRACE_EXITRC ( SDB_FORMATARGS, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_CREATEDAEMONPROC, "createDaemonProcess" )
INT32 createDaemonProcess ( const CHAR * program , const OSSPID & ppid ,
                             CHAR * f2dbuf , CHAR * d2fbuf )
{
   CHAR *         args     = NULL ;
   INT32          rc       = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_CREATEDAEMONPROC );
   OSSPID         pid ;
   ossResultCode  result ;
   OSSNPIPE       waitPipe ;
   CHAR           waitName[128] ;
   CHAR           f2dName[128] ;
   CHAR           d2fName[128] ;

   ossMemset ( &waitPipe , 0 , sizeof ( waitPipe ) ) ;
   ossMemset ( waitName , 0 , sizeof ( waitName ) ) ;
   ossMemset ( f2dName , 0 , sizeof ( f2dName ) ) ;
   ossMemset ( d2fName , 0 , sizeof ( d2fName ) ) ;

   SDB_ASSERT ( program && program[0] != '\0' , "Invalid argument" ) ;

   rc = getWaitPipeName ( ppid ,  waitName , sizeof ( waitName ) ) ;
   SH_VERIFY_RC

   // waitPipe is deleted in done:
   rc = ossCreateNamedPipe ( waitName , 0 , 0 , OSS_NPIPE_INBOUND ,
                             1 , 0 , waitPipe ) ;
   SH_VERIFY_RC

   // args is freed in done ;
   rc = formatArgs ( program , ppid , &args ) ;
   SH_VERIFY_RC

   rc = ossExec ( program , args , NULL , 0 , pid , result , NULL , NULL ) ;
   SH_VERIFY_RC

   rc = getPipeNames2 ( ppid , pid , f2dName , sizeof ( f2dName ) ,
                                     d2fName , sizeof ( d2fName ) ) ;
   SH_VERIFY_RC

   ossStrcpy ( f2dbuf , f2dName ) ;
   ossStrcpy ( d2fbuf , d2fName ) ;

   rc = ossConnectNamedPipe ( waitPipe , OSS_NPIPE_INBOUND ) ;
   SH_VERIFY_RC

   rc = ossDisconnectNamedPipe ( waitPipe ) ;
   SH_VERIFY_RC

done :
   ossDeleteNamedPipe ( waitPipe ) ;
   SAFE_OSS_FREE ( args ) ;
   PD_TRACE_EXITRC ( SDB_CREATEDAEMONPROC, rc );
   return rc ;
error :
   goto done ;
}

#define SDB_FRONTEND_RECEIVEBUFFERSIZE 128
PD_TRACE_DECLARE_FUNCTION ( SDB_ENTERFRONTENDMODE, "enterFrontEndMode" )
INT32 enterFrontEndMode ( const CHAR * program , const CHAR * cmd )
{
   CHAR     c           = '\0' ;
   INT32    rc          = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_ENTERFRONTENDMODE );
   OSSPID   ppid        = OSS_INVALID_PID ;
   OSSNPIPE f2dPipe ;
   OSSNPIPE d2fPipe ;
   CHAR     f2dName[128]          = {0} ;
   CHAR     d2fName[128]          = {0} ;
   CHAR     bpName[128]           = {0} ;
   CHAR     bpf2dName[128]        = {0} ;
   CHAR     bpd2fName[128]        = {0} ;
   CHAR     receiveBuffer1[SDB_FRONTEND_RECEIVEBUFFERSIZE]   = {0} ;
   CHAR     receiveBuffer2[SDB_FRONTEND_RECEIVEBUFFERSIZE]   = {0} ;
   CHAR     receiveBufferFinal[2*SDB_FRONTEND_RECEIVEBUFFERSIZE] = {0} ;
   CHAR    *pCurrentReceivePtr    = receiveBuffer1 ;
   CHAR *   p           = NULL ;
   INT32    id          = 0 ;

   ossMemset ( &f2dPipe , 0 , sizeof ( f2dPipe ) ) ;
   ossMemset ( &d2fPipe , 0 , sizeof ( d2fPipe ) ) ;

   ossMemset ( f2dName , 0 , sizeof ( f2dName ) ) ;
   ossMemset ( d2fName , 0 , sizeof ( d2fName ) ) ;
   ossMemset ( bpName , 0 , sizeof ( bpName ) ) ;
   ossMemset ( bpf2dName , 0 , sizeof ( bpf2dName ) ) ;
   ossMemset ( bpd2fName , 0 , sizeof ( bpd2fName ) ) ;

   SDB_ASSERT ( program && program[0] != '\0' , "invalid argument" ) ;
   if ( !cmd || cmd[0] == '\0' )
   {
      goto done ;
   }

   ppid = ossGetParentProcessID () ;
   SH_VERIFY_COND ( ppid != OSS_INVALID_PID , SDB_SYS ) ;

   rc = getPipeNames ( ppid , f2dName , sizeof ( f2dName ) ,
                       d2fName , sizeof ( d2fName ) ) ;
   SH_VERIFY_RC

   rc = getPipeNames1 ( bpf2dName , sizeof ( bpf2dName ) ,
                        bpd2fName , sizeof ( bpd2fName ) ,
                        f2dName , d2fName ) ;

   if ( rc == SDB_OK )
   {
      // the second parameter 45 is ascii value of '-'
      p = ossStrrchr ( bpf2dName , 45 ) ;
      p = p + 1 ;
      while ( *p != '\0' )
      {
         id = id * 10 ;
         id += ( INT32 )( *p - '0' ) ;
         p++ ;
      }
      if ( ossIsProcessRunning  ( (OSSPID)id ) )
      {
         rc = ossOpenNamedPipe ( bpf2dName , OSS_NPIPE_OUTBOUND , 0 , f2dPipe ) ;
         SH_VERIFY_RC
      }
      else
      {
         // first we should delete the old pipes
         rc = ossCleanNamedPipeByName ( bpf2dName ) ;
         SH_VERIFY_RC
         rc = ossCleanNamedPipeByName ( bpd2fName ) ;
         SH_VERIFY_RC

         // which will create those named pipes
         rc = ossLocateExecutable ( program , "sdbbp" , bpName , sizeof(bpName) ) ;
         SH_VERIFY_RC

         rc = createDaemonProcess ( bpName , ppid , bpf2dName , bpd2fName ) ;
         SH_VERIFY_RC

         rc = ossOpenNamedPipe ( bpf2dName , OSS_NPIPE_OUTBOUND , 0 , f2dPipe ) ;
         SH_VERIFY_RC
      }
   }
   else if ( rc == SDB_FNE )
   {
         // named pipe does not exist, so we need to create the daemon process
         // which will create those named pipes
         rc = ossLocateExecutable ( program , "sdbbp" , bpName , sizeof(bpName) ) ;
         SH_VERIFY_RC

         rc = createDaemonProcess ( bpName , ppid , bpf2dName , bpd2fName ) ;
         SH_VERIFY_RC

         rc = ossOpenNamedPipe ( bpf2dName , OSS_NPIPE_OUTBOUND , 0 , f2dPipe ) ;
         SH_VERIFY_RC
   }
   else
   {
      SH_VERIFY_RC
   }

   // also write the trailing \0 to mark end of write
   rc = ossWriteNamedPipe ( f2dPipe , cmd , ossStrlen ( cmd ) , NULL ) ;
   SH_VERIFY_RC

   rc = ossCloseNamedPipe ( f2dPipe ) ;
   SH_VERIFY_RC

   rc = ossOpenNamedPipe ( bpd2fName , OSS_NPIPE_INBOUND ,
                              OSS_NPIPE_INFINITE_TIMEOUT , d2fPipe ) ;
   SH_VERIFY_RC

   // read until hitting first ' ' for return code
/*
   rcBufferCount = 0 ;
   while ( SDB_OK == rc && (rcBufferCount < sizeof(returnCodeBuffer)-1))
   {
      rc = ossReadNamedPipe ( d2fPipe , &c , 1 , NULL ) ;
      // if we read the first space, that means we already read the return code
      // so let's jump out of here
      if ( ' ' == c )
         break ;
      returnCodeBuffer[rcBufferCount] = c ;
      ++rcBufferCount ;
   }

   // make sure we get right format, if we hit max buffer length,
   // or we read the full line without hitting space, the format
   // must be wrong, in this case let's print out whatever we received, and then
   // run the follow-up loop to get all output
   if ( rc || ( rcBufferCount == sizeof(returnCodeBuffer)-1 ) )
   {
      returnCodeBuffer[rcBufferCount] = '\0' ;
      ossPrintf ( "%s", returnCodeBuffer ) ;
   }
   else
   {
      // if we successfully received something, let's try to convert it into
      // return code and set it globally
      gShellReturnCode = ossAtoi ( returnCodeBuffer ) ;
   }
*/
   // rest are the actual message
   // if we failed at first loop, we'll never enter here since rc != SDB_OK
   ossMemset ( receiveBuffer1, 0, SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
   ossMemset ( receiveBuffer2, 0, SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
   while ( TRUE )
   {
      rc = ossReadNamedPipe ( d2fPipe , &c , 1 , NULL ) ;
      // loop until reading something
      if ( rc )
         break ;
      if ( ( pCurrentReceivePtr - &receiveBuffer1[0] <
             SDB_FRONTEND_RECEIVEBUFFERSIZE-1 &&
             pCurrentReceivePtr >= &receiveBuffer1[0] ) ||
           ( pCurrentReceivePtr - &receiveBuffer2[0] <
             SDB_FRONTEND_RECEIVEBUFFERSIZE-1 &&
             pCurrentReceivePtr >= &receiveBuffer2[0] ) )
      {
         // if we are in buffer 1 or buffer 2
         *pCurrentReceivePtr = c ;
         ++pCurrentReceivePtr ;
      }
      else if ( pCurrentReceivePtr - &receiveBuffer1[0] ==
                SDB_FRONTEND_RECEIVEBUFFERSIZE-1 )
      {
         // if we are at end of buffer 1, let's dump buffer 2 and then clear the
         // buffer
         // note we should NOT dump buffer 1 at the moment since we need to
         // extract the rc at the end
         receiveBuffer2[SDB_FRONTEND_RECEIVEBUFFERSIZE-1] = '\0' ;
         ossPrintf ( "%s", receiveBuffer2 ) ;
         ossMemset ( receiveBuffer2, 0, SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
         pCurrentReceivePtr = &receiveBuffer2[0] ;
         *pCurrentReceivePtr = c ;
         ++pCurrentReceivePtr ;
      }
      else if ( pCurrentReceivePtr - &receiveBuffer2[0] ==
                SDB_FRONTEND_RECEIVEBUFFERSIZE-1 )
      {
         // if we are at end of buffer 1, let's dump buffer 2 and then clear the
         // buffer
         // note we should NOT dump buffer 1 at the moment since we need to
         // extract the rc at the end
         receiveBuffer1[SDB_FRONTEND_RECEIVEBUFFERSIZE-1] = '\0' ;
         ossPrintf ( "%s", receiveBuffer1 ) ;
         ossMemset ( receiveBuffer1, 0, SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
         pCurrentReceivePtr = &receiveBuffer1[0] ;
         *pCurrentReceivePtr = c ;
         ++pCurrentReceivePtr ;
      }
      else
      {
         // something wrong, we should never hit here
         ossPrintf ( "SEVERE Error, we should never hit here"OSS_NEWLINE ) ;
         rc = SDB_SYS ;
         gShellReturnCode = SDB_RETURNCODE_SYSTEM ;
         goto error ;
      }
   }
   // after we receive buffer, let's scan the buffer
   // the rule is simple, let's copy the one not currently writing, then the
   // buffer we are currently writting to
   if ( pCurrentReceivePtr - &receiveBuffer1[0] <
        SDB_FRONTEND_RECEIVEBUFFERSIZE-1 &&
        pCurrentReceivePtr >= &receiveBuffer1[0] )
   {
      ossStrncpy ( receiveBufferFinal, receiveBuffer2,
                   SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
      ossStrncat ( receiveBufferFinal, receiveBuffer1,
                   SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
   }
   else if ( pCurrentReceivePtr - &receiveBuffer2[0] <
             SDB_FRONTEND_RECEIVEBUFFERSIZE-1 &&
             pCurrentReceivePtr >= &receiveBuffer2[0] )
   {
      ossStrncpy ( receiveBufferFinal, receiveBuffer1,
                   SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
      ossStrncat ( receiveBufferFinal, receiveBuffer2,
                   SDB_FRONTEND_RECEIVEBUFFERSIZE ) ;
   }
   pCurrentReceivePtr = &receiveBufferFinal[ossStrlen(receiveBufferFinal)] ;
   while ( pCurrentReceivePtr != &receiveBufferFinal[0] &&
           *pCurrentReceivePtr != ' ' )
   {
      --pCurrentReceivePtr ;
   }
   if ( *pCurrentReceivePtr == ' ' )
   {
      gShellReturnCode = ossAtoi ( pCurrentReceivePtr+1 ) ;
   }
   *pCurrentReceivePtr = '\0' ;
   ossPrintf ( "%s" , receiveBufferFinal ) ;
   SH_VERIFY_COND ( SDB_OK == rc || SDB_EOF == rc , rc )

   rc = ossCloseNamedPipe( d2fPipe ) ;
   SH_VERIFY_RC

done :
   PD_TRACE_EXITRC ( SDB_ENTERFRONTENDMODE, rc );
   return rc ;
error :
   ossCloseNamedPipe ( f2dPipe ) ;
   ossCloseNamedPipe ( d2fPipe ) ;
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDB_MAIN, "main" )
int main ( int argc , CHAR **argv )
{
   ScriptEngine *    engine   = NULL ;
   Scope *           scope    = NULL ;
   INT32             rc       = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDB_MAIN );
   ArgInfo           argInfo ;

   linenoiseSetCompletionCallback( (linenoiseCompletionCallback*)lineComplete ) ;

   // will purge engine in done:
   engine = ScriptEngine::globalScriptEngine() ;
   SH_VERIFY_COND ( engine , SDB_SYS )

   // scope is freed in done:
   scope = engine->newScope() ;
   SH_VERIFY_COND ( scope , SDB_SYS )

   // parse Argument into argInfo
   rc = parseArguments ( argc , argv , argInfo ) ;
   if( SDB_SDB_HELP_ONLY == rc || SDB_SDB_VERSION_ONLY == rc )
   {
      rc = SDB_OK ;
      goto done ;
   }
   SH_VERIFY_RC

   switch ( argInfo.mode )
   {
   case INTERACTIVE_MODE :
      rc = enterInteractiveMode( scope ) ;
      break ;

   case BATCH_MODE :
      rc = enterBatchMode( scope , argInfo.filename , argInfo.variable ) ;
      break ;

   case FRONTEND_MODE :
      rc = enterFrontEndMode ( argInfo.program , argInfo.cmd ) ;
      break ;

   default :
      rc = SDB_INVALIDARG ;
   }

done :
   SAFE_OSS_DELETE ( scope ) ;
   ScriptEngine::purgeGlobalScriptEngine() ;
   PD_TRACE_EXITRC ( SDB_SDB_MAIN, rc );
   if ( rc )
   {
      // if rc is here, that means something really goes off in either
      // javascript engine or pipe
      gShellReturnCode = SDB_RETURNCODE_SYSTEM ;
   }
   return gShellReturnCode ;
error :
   goto done ;
}
