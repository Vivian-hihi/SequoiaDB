/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sdbimprt.cpp

   Descriptive Name = Import Utility

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbimprt
   which is used to do data import

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/21/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include <string>
#include <iostream>

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#include "core.hpp"
#include "migImportLoad.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "ossSocket.hpp"
#include "ossSignal.hpp"
#include "ossPrimitiveFileOp.hpp"
#include "ossStackDump.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "msgDef.h"
#if defined (_LINUX)
#include <execinfo.h>
#endif
using namespace std ;
namespace po = boost::program_options ;

extern CHAR _pdDiagLogPath[OSS_MAX_PATHSIZE+1] ;
#define LOGPATH "sdbimport.log"

#define OPTION_HELP        "help"
#define OPTION_HOSTNAME    "hostname"
#define OPTION_SVCNAME     "svcname"
#define OPTION_DELCHAR     "delchar"
#define OPTION_DELFIELD    "delfield"
#define OPTION_DELRECORD   "delrecord"
#define OPTION_MONGO       "mongo"
#define OPTION_FILENAME    "file"
#define OPTION_LIBPATH     "libpath"
#define OPTION_EXTRA       "extra"
#define OPTION_SPARSE      "sparse"
#define OPTION_MODEL       "model"
#define OPTION_SOURCEHOST  "sourcehost"
#define OPTION_SOURCEPORT  "sourceport"
#define OPTION_SOURCEUSER  "sourceuser"
#define OPTION_SOURCEPWD   "sourcepassword"
#define OPTION_INSERTNUM   "insertnum"

#define OPTION_FIELD             FIELD_NAME_FIELDS
#define OPTION_HEADERLINE        FIELD_NAME_HEADERLINE
#define OPTION_COLLECTSPACE      "csname"
#define OPTION_COLLECTION        "clname"
#define OPTION_TYPE              FIELD_NAME_LTYPE

#define DEFAULT_HOSTNAME   "localhost"
#define FIELD_SEPARATOR    ","

#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( OPTION_HELP, "help" )\
       ( COMMANDS_STRING(OPTION_HOSTNAME,       ",h"), boost::program_options::value<string>(), "database host name ( default: localhost )" ) \
       ( COMMANDS_STRING(OPTION_SVCNAME,        ",s"), boost::program_options::value<string>(), "database service name ( default: 50000 " ) \
       ( COMMANDS_STRING(OPTION_DELCHAR,        ",a"), boost::program_options::value<string>(), "string delimiter ( default: \" )( CSV type only )" ) \
       ( COMMANDS_STRING(OPTION_DELFIELD,       ",e"), boost::program_options::value<string>(), "field delimiter ( default: , )( CSV type only )" ) \
       ( COMMANDS_STRING(OPTION_DELRECORD,      ",r"), boost::program_options::value<string>(), "record delimiter ( default: '\\n' )( CSV type only )" ) \
       ( COMMANDS_STRING(OPTION_COLLECTSPACE,   ",c"), boost::program_options::value<string>(), "collection space name" ) \
       ( COMMANDS_STRING(OPTION_COLLECTION,     ",l"), boost::program_options::value<string>(), "collection name" ) \
       ( COMMANDS_STRING(OPTION_INSERTNUM,      ",n"), boost::program_options::value<string>(), "batch insert records number, default: 100" ) \
       ( OPTION_FILENAME,      boost::program_options::value<string>(), "database load file name" ) \
       ( OPTION_MODEL,         boost::program_options::value<string>(), "get data model, default: io ( io, hdfs )" ) \
       ( OPTION_LIBPATH,       boost::program_options::value<string>(), "load lib path ( hdfs model only )" ) \
       ( OPTION_SOURCEHOST,    boost::program_options::value<string>(), "connect to source data server, hostname ( hdfs model only )" ) \
       ( OPTION_SOURCEPORT,    boost::program_options::value<string>(), "connect to source data server, port ( hdfs model only )" ) \
       ( OPTION_SOURCEUSER,    boost::program_options::value<string>(), "connect to source data server, user ( hdfs model only )" ) \
       ( OPTION_TYPE,          boost::program_options::value<string>(), "type of file to load, default: json (json,csv)" ) \
       ( OPTION_FIELD,         boost::program_options::value<string>(), "comma separated list of field names e.g. --fields name,age" ) \
       ( OPTION_HEADERLINE,    boost::program_options::value<string>(), "first line in input file is a header, default: false ( CSV type only )" ) \
       ( OPTION_SPARSE,        boost::program_options::value<string>(), "auto add fields, default: true ( CSV type only )" ) \
       ( OPTION_EXTRA,         boost::program_options::value<string>(), "auto add value, default: false ( CSV type only )" )
       //( OPTION_SOURCEPWD,     boost::program_options::value<string>(), "connect to source data server, password" ) \
//       ( COMMANDS_STRING(OPTION_MONGO,          ",m"), boost::program_options::value<string>(), "Compatible with MongoDB data format, input [true, false]" )


CHAR *SDBIMPORT_TYPE_STR[] =
{
   "csv",
   "json"
} ;

CHAR *SDBIMPORT_MODEL_STR[] =
{
   "io",
   "hdfs"
} ;

CHAR gpHostName[OSS_MAX_HOSTNAME+1]       = {0} ;
CHAR gpServiceName[OSS_MAX_SERVICENAME+1] = {0} ;
migImportTypes gImportType                = MIG_IMPORT_TYPE_CSV ;
migImportAccess accessModel               = MIG_IMPORT_GET_IO ;
CHAR *gpInputFileName                     = NULL ;
CHAR *gPath                               = NULL ;
CHAR gSourceHostName[OSS_MAX_HOSTNAME+1]  = {0} ;
CHAR gSourcePort[OSS_MAX_SERVICENAME+1]   = {0} ;
CHAR *gScourceUser                        = NULL ;
CHAR  *strField                           = NULL ;
string strDelChar                         = "" ;
string strDelField                        = "" ;
string strDelRecord                       = "" ;
string strCSName                          = "" ;
string strCLName                          = "" ;
BOOLEAN bMongoCompatible                  = FALSE ;
BOOLEAN isHeaderline                      = FALSE ;
BOOLEAN autoAddField                      = TRUE  ;
BOOLEAN autoCompletion                    = FALSE ;
INT32 lInsertNum = 100 ;

CHAR gDelList[6] = { MIG_DEFAULT_DELCHAR, 0, MIG_DEFAULT_DELFIELD, 0,
                     MIG_DEFAULT_DELRECORD, 0 } ;

// global connection and collection
sdbConnectionHandle gConnection ;
sdbCSHandle gCollectionSpace ;
sdbCollectionHandle gCollection ;

static inline std::string &ltrim ( std::string &s )
{
   s.erase ( s.begin(), std::find_if ( s.begin(), s.end(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace)))) ;
   return s ;
}

static inline std::string &rtrim ( std::string &s )
{
   s.erase ( std::find_if ( s.rbegin(), s.rend(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace))).base(),
             s.end() ) ;
   return s ;
}

static inline std::string &trim ( std::string &s )
{
   return ltrim ( rtrim ( s ) ) ;
}

#if defined (_LINUX)

PD_TRACE_DECLARE_FUNCTION ( SDB_MIGLOADTRAPHNDL, "migTrapHandler" )

void migTrapHandler ( OSS_HANDPARMS )
{
   PD_TRACE_ENTRY ( SDB_MIGLOADTRAPHNDL );
   CHAR dumpDir[OSS_MAX_PATHSIZE+1] = "./" ;
   if ( signum == OSS_STACK_DUMP_SIGNAL ||
        signum == OSS_STACK_DUMP_SIGNAL_INTERNAL )
   {
      PD_LOG ( PDEVENT,
               "Signal %d is received, "
               "prepare to dump stack for %u:%u", signum,
               ossGetCurrentProcessID(),
               ossGetCurrentThreadID() ) ;
      void *pSyms[1] ;
      CHAR fileName[OSS_MAX_PATHSIZE] = {0} ;
      ossPrimitiveFileOp trapFile ;
      UINT32 strLen = 0 ;
      ossSnprintf ( fileName, OSS_MAX_PATHSIZE, "%d.%d.trap",
                    ossGetCurrentProcessID(),
                    ossGetCurrentThreadID() ) ;
      if ( ossStrlen ( dumpDir ) + ossStrlen ( OSS_PRIMITIVE_FILE_SEP ) +
           ossStrlen ( fileName ) > OSS_MAX_PATHSIZE )
      {
         pdLog ( PDERROR, __FUNC__, __FILE__, __LINE__,
                 "path + file name is too long" ) ;
         goto error ;
      }


      ossMemset( fileName, 0, sizeof( fileName ) ) ;
      strLen += ossSnprintf( fileName, sizeof( fileName ), "%s%s",
                             dumpDir, OSS_PRIMITIVE_FILE_SEP ) ;
      ossSnprintf( fileName + strLen, sizeof(fileName) - strLen,
                   "%d.%d.trap",
                   ossGetCurrentProcessID(),
                   ossGetCurrentThreadID() ) ;

      backtrace( pSyms, 1 ) ;
      trapFile.Open( fileName ) ;

      if ( trapFile.isValid() )
      {
         trapFile.seekToEnd () ;
         ossDumpStackTrace( OSS_HANDARGS, &trapFile ) ;
      }

      trapFile.Close() ;
   }
   else
   {
      PD_LOG ( PDWARNING, "Unexpected signal is received: %d",
               signum ) ;
   }
done :
   PD_TRACE_EXIT ( SDB_MIGLOADTRAPHNDL );
   return ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_MIGLOADSSH, "migSetupSignalHandler" )
INT32 migSetupSignalHandler()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_MIGLOADSSH );
   struct sigaction newact ;
   ossMemset ( &newact, 0, sizeof(newact) ) ;
   sigemptyset ( &newact.sa_mask ) ;
   newact.sa_sigaction = ( OSS_SIGFUNCPTR ) migTrapHandler ;
   newact.sa_flags |= SA_SIGINFO ;
   newact.sa_flags |= SA_ONSTACK ;
   if ( sigaction ( OSS_STACK_DUMP_SIGNAL, &newact, NULL ) )
   {
      PD_LOG ( PDERROR, "Failed to setup signal handler for dump signal" ) ;
      rc = SDB_SYS ;
      goto error ;
   }
   if ( sigaction ( OSS_STACK_DUMP_SIGNAL_INTERNAL, &newact, NULL ) )
   {
      PD_LOG ( PDERROR, "Failed to setup signal handler for dump signal" ) ;
      rc = SDB_SYS ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB_MIGLOADSSH, rc );
   return rc ;
error :
   goto done ;
}
#endif


void init ( po::options_description &desc )
{
   ADD_PARAM_OPTIONS_BEGIN ( desc )
      COMMANDS_OPTIONS
   ADD_PARAM_OPTIONS_END
}

void displayArg ( po::options_description &desc )
{
   std::cout << desc << std::endl ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBIMPLOAD_RESOLVEARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBIMPLOAD_RESOLVEARG );
   po::variables_map vm ;
   const CHAR *pFileName = NULL ;
   const CHAR *pField = NULL ;
   const CHAR *pPath  = NULL ;
   const CHAR *pUser  = NULL ;
   try
   {
      po::store ( po::parse_command_line ( argc, argv, desc ), vm ) ;
      po::notify ( vm ) ;
   }
   catch ( po::unknown_option &e )
   {
      pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
            ( ( std::string ) "Unknown argument: " +
                e.get_option_name ()).c_str () ) ;
              std::cerr <<  "Unknown argument: "
                        << e.get_option_name () << std::endl ;
              rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch ( po::invalid_option_value &e )
   {
      pdLog ( PDWARNING, __FUNC__, __FILE__, __LINE__,
             ( ( std::string ) "Invalid argument: " +
               e.get_option_name () ).c_str () ) ;
      std::cerr <<  "Invalid argument: "
                << e.get_option_name () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   catch( po::error &e )
   {
      std::cerr << e.what () << std::endl ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   if ( vm.count ( OPTION_HELP ) )
   {
      displayArg ( desc ) ;
      rc = SDB_PMD_HELP_ONLY ;
      goto done ;
   }

   // hostname is optional, default 127.0.0.1
   if ( vm.count ( OPTION_HOSTNAME ) )
   {
      ossStrncpy ( gpHostName, vm[OPTION_HOSTNAME].as<string>().c_str(),
                   OSS_MAX_HOSTNAME ) ;
   }
   else
   {
      ossStrncpy ( gpHostName, DEFAULT_HOSTNAME, OSS_MAX_HOSTNAME ) ;
   }

   // service name is optional, default is OSS_DFT_SVCPORT, which is 50000
   if ( vm.count ( OPTION_SVCNAME ) )
   {
      ossStrncpy ( gpServiceName, vm[OPTION_SVCNAME].as<string>().c_str(),
                   OSS_MAX_SERVICENAME ) ;
   }
   else
   {
      ossSnprintf ( gpServiceName, OSS_MAX_SERVICENAME, "%d",
                    OSS_DFT_SVCPORT ) ;
   }

   // import type is optional, default is csv
   if ( vm.count ( OPTION_TYPE ) )
   {
      for ( UINT32 i = 0; i < MIG_IMPORT_TYPE_END; ++i )
      {
         if ( ossStrncasecmp ( vm[OPTION_TYPE].as<string>().c_str(),
                               SDBIMPORT_TYPE_STR[i],
                               ossStrlen ( SDBIMPORT_TYPE_STR[i] ) + 1 ) == 0 )
         {
            gImportType = (migImportTypes)i ;
            break ;
         }
      }
   }

   // import model is optional, default is io
   if ( vm.count ( OPTION_MODEL ) )
   {
      for ( UINT32 i = 0; i < MIG_IMPORT_GET_END; ++i )
      {
         if ( ossStrncasecmp ( vm[OPTION_MODEL].as<string>().c_str(),
                               SDBIMPORT_MODEL_STR[i],
                               ossStrlen ( SDBIMPORT_MODEL_STR[i] ) + 1 ) == 0 )
         {
            accessModel = (migImportAccess)i ;
            break ;
         }
      }
   }

   // file name
   if ( vm.count ( OPTION_FILENAME ) )
   {
      pFileName = vm[OPTION_FILENAME].as<string>().c_str() ;
      INT32 bufSize = ossStrlen ( pFileName ) + 1 ;
      gpInputFileName = (CHAR*)SDB_OSS_MALLOC ( bufSize ) ;
      if ( !gpInputFileName )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                  bufSize ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossStrncpy ( gpInputFileName, pFileName, bufSize ) ;
   }
   else
   {
      PD_LOG ( PDERROR, "file name must input" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   // field list
   if ( vm.count ( OPTION_FIELD ) )
   {
      pField = vm[OPTION_FIELD].as<string>().c_str() ;
      INT32 bufSize = ossStrlen ( pField ) + 1 ;
      strField = (CHAR*)SDB_OSS_MALLOC ( bufSize ) ;
      if ( !strField )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                  bufSize ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossStrncpy ( strField, pField, bufSize ) ;
   }

   //read first line?
   if ( vm.count ( OPTION_HEADERLINE ) )
   {
      ossStrToBoolean ( vm[OPTION_HEADERLINE].as<string>().c_str(),
                        &isHeaderline ) ;
   }

   if ( vm.count ( OPTION_SPARSE ) )
   {
      ossStrToBoolean ( vm[OPTION_SPARSE].as<string>().c_str(),
                        &autoAddField ) ;
   }

   if ( vm.count ( OPTION_EXTRA ) )
   {
      ossStrToBoolean ( vm[OPTION_EXTRA].as<string>().c_str(),
                        &autoCompletion ) ;
   }

   // Compatible MongoDB
   /*if ( vm.count ( OPTION_MONGO ) )
   {
      ossStrToBoolean ( vm[OPTION_MONGO].as<string>().c_str(),
                        &bMongoCompatible ) ;
   }*/

   // del char, by default '"'
   if ( vm.count ( OPTION_DELCHAR ) )
   {
      strDelChar = vm[OPTION_DELCHAR].as<string>() ;
   }

   // del field , by default ','
   if ( vm.count ( OPTION_DELFIELD ) )
   {
      strDelField = vm[OPTION_DELFIELD].as<string>() ;
   }

   // del record, by default '\n'
   if ( vm.count ( OPTION_DELRECORD ) )
   {
      strDelRecord = vm[OPTION_DELRECORD].as<string>() ;
   }

   // collection space name
   if ( vm.count ( OPTION_COLLECTSPACE ) )
   {
      strCSName = vm[OPTION_COLLECTSPACE].as<string>() ;
   }
   // collection name
   if ( vm.count ( OPTION_COLLECTION ) )
   {
      strCLName = vm[OPTION_COLLECTION].as<string>() ;
   }

   // lib path
   if ( vm.count ( OPTION_LIBPATH ) )
   {
      pPath = vm[OPTION_LIBPATH].as<string>().c_str() ;
      INT32 bufSize = ossStrlen ( pPath ) + 1 ;
      gPath = (CHAR*)SDB_OSS_MALLOC ( bufSize ) ;
      if ( !gPath )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                  bufSize ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossStrncpy ( gPath, pPath, bufSize ) ;
   }

   // source user
   if ( vm.count ( OPTION_SOURCEUSER ) )
   {
      pUser = vm[OPTION_SOURCEUSER].as<string>().c_str() ;
      INT32 bufSize = ossStrlen ( pPath ) + 1 ;
      gScourceUser = (CHAR*)SDB_OSS_MALLOC ( bufSize ) ;
      if ( !gScourceUser )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                  bufSize ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossStrncpy ( gScourceUser, pUser, bufSize ) ;
   }

   // source hostname
   if ( vm.count ( OPTION_SOURCEHOST ) )
   {
      ossStrncpy ( gSourceHostName, vm[OPTION_SOURCEHOST].as<string>().c_str(),
                   OSS_MAX_HOSTNAME ) ;
   }

   // source port
   if ( vm.count ( OPTION_SOURCEPORT ) )
   {
      ossStrncpy ( gSourcePort, vm[OPTION_SOURCEPORT].as<string>().c_str(),
                   OSS_MAX_SERVICENAME ) ;
   }

   if ( vm.count ( OPTION_INSERTNUM ) )
   {
      lInsertNum = ossAtoi ( vm[OPTION_INSERTNUM].as<string>().c_str() ) ;
      if ( lInsertNum <= 0 )
      {
         rc = SDB_INVALIDARG ;
         PD_LOG ( PDERROR, "insert records number must be greater than 100" ) ;
         goto error ;
      }
   }
done :
   PD_TRACE_EXITRC ( SDB_SDBIMPLOAD_RESOLVEARG, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBIMPLOAD_GETCOLLECTION, "getCollection" )
INT32 getCollection ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBIMPLOAD_GETCOLLECTION );
   if ( !strCSName.length() )
   {
      ossPrintf ( "Collection Space name must be specified"OSS_NEWLINE ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   if ( !strCLName.length() )
   {
      ossPrintf ( "Collection name must be specified"OSS_NEWLINE ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // connection is established
   rc = sdbConnect ( gpHostName, gpServiceName, "", "", &gConnection ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to connect to database %s:%s, rc = %d",
                  gpHostName, gpServiceName, rc ) ;
      goto error ;
   }

   // get collection space
   rc = sdbGetCollectionSpace ( gConnection, strCSName.c_str(),
                                &gCollectionSpace ) ;
   if ( SDB_DMS_CS_NOTEXIST == rc )
   {
      ossPrintf ( "Collection space %s does not exist"OSS_NEWLINE,
                  strCSName.c_str() ) ;
      goto error ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection space %s, rc = %d",
               strCSName.c_str(), rc ) ;
      goto error ;
   }

   // get collection
   rc = sdbGetCollection1 ( gCollectionSpace, strCLName.c_str(),
                            &gCollection ) ;
   if ( SDB_DMS_NOTEXIST == rc )
   {
      ossPrintf ( "Collection %s does not exist"OSS_NEWLINE,
                  strCLName.c_str() ) ;
      goto error ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection %s, rc = %d",
               strCLName.c_str(), rc ) ;
      goto error ;
   }
done :
   PD_TRACE_EXITRC ( SDB_SDBIMPLOAD_GETCOLLECTION, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_IMPORTLOADCSV, "importCSV" )
INT32 importCSV ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_IMPORTLOADCSV );
   INT32 total = 0, succ = 0 ;
   migCSVParser *parser = NULL ;

   if ( !isHeaderline && !strField )
   {
      PD_LOG ( PDERROR, "if not read first line,than must input fields" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   parser = SDB_OSS_NEW _migCSVParser () ;
   if ( !parser )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for parser" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   // initialize
   rc = parser->init ( gCollection, gpInputFileName,
                       gPath,
                       strDelChar.length()?strDelChar.c_str():NULL,
                       strDelField.length()?strDelField.c_str():NULL,
                       strDelRecord.length()?strDelRecord.c_str():NULL,
                       gSourceHostName,
                       gScourceUser,
                       (UINT16)ossAtoi(gSourcePort),
                       strField, isHeaderline,
                       autoAddField, autoCompletion,
                       accessModel ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize parser, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = parser->run ( total, succ, lInsertNum ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute parser, rc = %d", rc ) ;
      goto error ;
   }
   ossPrintf ( "%d records in CSV file, %d records import"OSS_NEWLINE, total, succ ) ;
done :
   if ( parser )
      SDB_OSS_DEL parser ;
   PD_TRACE_EXITRC ( SDB_IMPORTLOADCSV, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_IMPORTLOADJSON, "importJson" )
INT32 importJson ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_IMPORTLOADJSON );
   INT32 total = 0, succ = 0 ;
   migJSONParser *parser = NULL ;
   parser = SDB_OSS_NEW _migJSONParser () ;
   if ( !parser )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for parser" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   // initialize
   rc = parser->init ( gCollection,
                       gpInputFileName,
                       gPath,
                       gSourceHostName,
                       gScourceUser,
                       (UINT16)ossAtoi(gSourcePort),
                       bMongoCompatible,
                       accessModel ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize parser, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = parser->run ( total, succ, lInsertNum ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute parser, rc = %d", rc ) ;
      goto error ;
   }
   ossPrintf ( "%d records in Json file, %d records import"OSS_NEWLINE, total, succ ) ;
done :
   if ( parser )
   {
      SDB_OSS_DEL parser ;
   }
   PD_TRACE_EXITRC ( SDB_IMPORTLOADJSON, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBIMPLOAD_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBIMPLOAD_MAIN );

   // set log path
   ossMemset ( _pdDiagLogPath, 0, sizeof( _pdDiagLogPath ) ) ;
   ossStrncpy ( _pdDiagLogPath, LOGPATH, sizeof(LOGPATH) ) ;

   po::options_description desc ( "Command options" ) ;
   init ( desc ) ;
   rc = resolveArgument ( desc, argc, argv ) ;
   if ( rc )
   {
      if ( SDB_PMD_HELP_ONLY != rc )
      {
         PD_LOG ( PDERROR, "Invalid argument" ) ;
         displayArg ( desc ) ;
      }
      goto done ;
   }
#if defined (_LINUX)
   // signal handler
   rc = migSetupSignalHandler () ;
   if ( rc )
      goto error ;
#endif
   // attempt to connect to database and query from collection
   rc = getCollection () ;
   if ( rc )
   {
      //PD_LOG ( PDERROR, "Failed to get collection, rc = %d", rc ) ;
      goto error ;
   }
   switch ( gImportType )
   {
   case MIG_IMPORT_TYPE_CSV :
      rc = importCSV () ;
      break ;
   case MIG_IMPORT_TYPE_JSON :
      rc = importJson () ;
      break ;
   default :
      PD_LOG ( PDERROR, "Invalid type" ) ;
      goto error ;
   }
done :
   if ( rc )
   {
      if ( rc != SDB_PMD_HELP_ONLY )
      {
         ossPrintf ( "Import Failed"OSS_NEWLINE ) ;
      }
   }
   else
   {
      ossPrintf ( "Import Successfully"OSS_NEWLINE ) ;
   }
   if ( rc != SDB_PMD_HELP_ONLY )
   {
      ossPrintf ( "Detail in log path: %s"OSS_NEWLINE, _pdDiagLogPath ) ;
   }
   sdbDisconnect ( gConnection ) ;
   if ( gCollection )
   {
      sdbReleaseCollection ( gCollection ) ;
   }
   if ( gCollectionSpace )
   {
      sdbReleaseCS ( gCollectionSpace ) ;
   }
   if ( gConnection )
   {
      sdbReleaseConnection ( gConnection ) ;
   }
   if ( gpInputFileName )
      SDB_OSS_FREE ( gpInputFileName ) ;
   if ( strField )
      SDB_OSS_FREE ( strField ) ;
   PD_TRACE_EXITRC ( SDB_SDBIMPLOAD_MAIN, rc );
   return rc ;
error :
   goto done ;
}
