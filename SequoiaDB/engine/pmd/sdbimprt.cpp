/*******************************************************************************


   Copyright (C) 2011-2014 SequoiaDB Ltd.

   This program is free software: you can redistribute it and/or modify
   it under the term of the GNU Affero General Public License, version 3,
   as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warrenty of
   MARCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with this program. If not, see <http://www.gnu.org/license/>.

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
#include "core.hpp"
#include "migImport.hpp"
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

#include <string>
#include <iostream>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

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
#define OPTION_USER        "user"
#define OPTION_PASSWORD    "password"
#define OPTION_DELCHAR     "delchar"
#define OPTION_DELFIELD    "delfield"
#define OPTION_DELRECORD   "delrecord"
#define OPTION_MONGO       "mongo"
#define OPTION_FILENAME    "file"
#define OPTION_EXTRA       "extra"
#define OPTION_SPARSE      "sparse"
#define OPTION_LINEPRIORITY "linepriority"
#define OPTION_STRINGTYPE  "stringtype"

#define OPTION_FIELD             FIELD_NAME_FIELDS
#define OPTION_HEADERLINE        FIELD_NAME_HEADERLINE
#define OPTION_COLLECTSPACE      "csname"
#define OPTION_COLLECTION        "clname"
#define OPTION_TYPE              FIELD_NAME_LTYPE

#define DEFAULT_HOSTNAME         "localhost"
#define FIELD_SEPARATOR          ","

#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( OPTION_HELP, "help" )\
       ( COMMANDS_STRING(OPTION_HOSTNAME,       ",h"), boost::program_options::value<string>(), "database host name") \
       ( COMMANDS_STRING(OPTION_SVCNAME,        ",s"), boost::program_options::value<string>(), "database service name" ) \
       ( COMMANDS_STRING(OPTION_USER,           ",u"), boost::program_options::value<string>(), "databse user" ) \
       ( COMMANDS_STRING(OPTION_PASSWORD,       ",w"), boost::program_options::value<string>(), "databse password" ) \
       ( COMMANDS_STRING(OPTION_DELCHAR,        ",a"), boost::program_options::value<string>(), "string delimiter ( default: \" )( csv only )" ) \
       ( COMMANDS_STRING(OPTION_DELFIELD,       ",e"), boost::program_options::value<string>(), "field delimiter ( default: , )( csv only )" ) \
       ( COMMANDS_STRING(OPTION_DELRECORD,      ",r"), boost::program_options::value<string>(), "record delimiter ( default: '\\n' )" ) \
       ( COMMANDS_STRING(OPTION_COLLECTSPACE,   ",c"), boost::program_options::value<string>(), "collection space name" ) \
       ( COMMANDS_STRING(OPTION_COLLECTION,     ",l"), boost::program_options::value<string>(), "collection name" ) \
       ( OPTION_FILENAME,      boost::program_options::value<string>(), "database load file name" ) \
       ( OPTION_TYPE,          boost::program_options::value<string>(), "type of file to load, default: json (json,csv)" ) \
       ( OPTION_FIELD,         boost::program_options::value<string>(), "comma separated list of field names e.g. --fields name,age ( csv only )" ) \
       ( OPTION_HEADERLINE,    boost::program_options::value<string>(), "first line in input file is a header, default: false ( csv only )" ) \
       ( OPTION_SPARSE,        boost::program_options::value<string>(), "auto add fields, default: true ( csv only )" ) \
       ( OPTION_EXTRA,         boost::program_options::value<string>(), "auto add value, default: false ( csv only )" ) \
       ( OPTION_LINEPRIORITY,  boost::program_options::value<string>(), "reverse the priority for record and character delimiter, default: true" ) \
       ( OPTION_STRINGTYPE,    boost::program_options::value<string>(), "all fields convert to string type, default: false" )

//       ( COMMANDS_STRING(OPTION_MONGO,          ",m"), boost::program_options::value<string>(), "Compatible with MongoDB data format, input [true, false]" )
enum SDBIMPORT_TYPE
{
   IMPORT_TYPE_CSV = 0,
   IMPORT_TYPE_JSON,
   IMPORT_TYPE_FINISH
} ;

CHAR *SDBIMPORT_TYPE_STR[] =
{
   "csv",
   "json"
} ;

CHAR gpHostName[OSS_MAX_HOSTNAME+1]       = {0} ;
CHAR gpServiceName[OSS_MAX_SERVICENAME+1] = {0} ;
SDBIMPORT_TYPE gImportType                = IMPORT_TYPE_CSV ;
CHAR *gpInputFileName                     = NULL ;
CHAR  *strField                           = NULL ;
string strDelChar                         = "" ;
string strDelField                        = "" ;
string strDelRecord                       = "" ;
string strCSName                          = "" ;
string strCLName                          = "" ;
CHAR *lUser                               = NULL ;
CHAR *lPassWord                           = NULL ;
BOOLEAN bMongoCompatible                  = FALSE ;
BOOLEAN isHeaderline                      = FALSE ;
BOOLEAN autoAddField                      = TRUE  ;
BOOLEAN autoCompletion                    = FALSE ;
BOOLEAN linePriority                      = TRUE ;
BOOLEAN stringType                        = FALSE ;

CHAR gDelList[6] = { MIG_DEFAULT_DELCHAR, 0, MIG_DEFAULT_DELFIELD, 0,
                     MIG_DEFAULT_DELRECORD, 0 } ;

// global connection and collection
sdbConnectionHandle gConnection ;
sdbCSHandle gCollectionSpace ;
sdbCollectionHandle gCollection ;

static OSS_INLINE std::string &ltrim ( std::string &s )
{
   s.erase ( s.begin(), std::find_if ( s.begin(), s.end(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace)))) ;
   return s ;
}

static OSS_INLINE std::string &rtrim ( std::string &s )
{
   s.erase ( std::find_if ( s.rbegin(), s.rend(),
             std::not1 ( std::ptr_fun<int, int>(std::isspace))).base(),
             s.end() ) ;
   return s ;
}

static OSS_INLINE std::string &trim ( std::string &s )
{
   return ltrim ( rtrim ( s ) ) ;
}

#if defined (_LINUX)
PD_TRACE_DECLARE_FUNCTION ( SDB_MIGTRAPHNDL, "migTrapHandler" )
void migTrapHandler ( OSS_HANDPARMS )
{
   PD_TRACE_ENTRY ( SDB_MIGTRAPHNDL );
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
   PD_TRACE_EXIT ( SDB_MIGTRAPHNDL );
   return ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_MIGSSH, "migSetupSignalHandler" )
INT32 migSetupSignalHandler()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_MIGSSH );
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
   PD_TRACE_EXITRC ( SDB_MIGSSH, rc );
   return rc ;
error :
   goto done ;
}
#endif
// check whether the file name includes suffix
/*BOOLEAN suffixIncluded ( const CHAR *pFileName, const CHAR *pSuffix )
{
   SDB_ASSERT ( pFileName, "file name can't be NULL" )
   SDB_ASSERT ( pSuffix, "suffix can't be NULL" )
   // get length of file name and suffix name
   INT32 fileNameLen = ossStrlen ( pFileName ) ;
   INT32 suffixLen = ossStrlen ( pSuffix ) ;
   const CHAR *pLastFileName = NULL ;
   const CHAR *pLastSuffix = NULL ;
   if ( fileNameLen == 0 || suffixLen == 0 )
      return FALSE ;
   // start from end of each string
   pLastFileName = &pFileName [ fileNameLen - 1 ] ;
   pLastSuffix = &pSuffix [ suffixLen - 1 ] ;
   // loop from back until one string running out
   while ( fileNameLen >= 0 && suffixLen >= 0 )
   {
      if ( *pLastFileName != *pLastSuffix )
         return FALSE ;
      // reduce counter
      --fileNameLen ;
      --suffixLen ;
      // move pointer back
      --pLastFileName ;
      --pLastSuffix ;
   }
   // only true when suffix is running out
   return suffixLen == 0 ;
}*/

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

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBIMP_RESOLVEARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBIMP_RESOLVEARG );
   po::variables_map vm ;
   const CHAR *pFileName = NULL ;
   const CHAR *pField = NULL ;
   const CHAR *pUser     = NULL ;
   const CHAR *pPassWord = NULL ;
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

   // service name is optional, default is OSS_DFT_SVCPORT, which is 11810
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

   if ( vm.count ( OPTION_USER ) )
   {
      pUser = vm[OPTION_USER].as<string>().c_str() ;
      lUser = (CHAR *)SDB_OSS_MALLOC ( ossStrlen ( pUser ) + 1 ) ;
      lUser[ossStrlen ( pUser )] = 0 ;
      ossStrncpy ( lUser, pUser, ossStrlen ( pUser ) ) ;
   }
   else
   {
      lUser = (CHAR *)SDB_OSS_MALLOC ( 1 ) ;
      *lUser = '\0' ;
   }

   if ( vm.count ( OPTION_PASSWORD ) )
   {
      pPassWord = vm[OPTION_PASSWORD].as<string>().c_str() ;
      lPassWord= (CHAR *)SDB_OSS_MALLOC ( ossStrlen ( pPassWord ) + 1 ) ;
      lPassWord[ossStrlen ( pPassWord )] = 0 ;
      ossStrncpy ( lPassWord, pPassWord, ossStrlen ( pPassWord ) ) ;
   }
   else
   {
      lPassWord = (CHAR *)SDB_OSS_MALLOC ( 1 ) ;
      *lPassWord = '\0' ;
   }

   // import type is optional, default is csv
   if ( vm.count ( OPTION_TYPE ) )
   {
      for ( UINT32 i = 0; i < IMPORT_TYPE_FINISH; ++i )
      {
         if ( ossStrncasecmp ( vm[OPTION_TYPE].as<string>().c_str(),
                               SDBIMPORT_TYPE_STR[i],
                               ossStrlen ( SDBIMPORT_TYPE_STR[i] ) + 1 ) == 0 )
         {
            gImportType = (SDBIMPORT_TYPE)i ;
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

   if ( vm.count ( OPTION_LINEPRIORITY ) )
   {
      ossStrToBoolean ( vm[OPTION_LINEPRIORITY].as<string>().c_str(),
                        &linePriority ) ;
   }

   //string type
   if ( vm.count ( OPTION_STRINGTYPE ) )
   {
      ossStrToBoolean ( vm[OPTION_STRINGTYPE].as<string>().c_str(),
                        &stringType ) ;
   }

done :
   PD_TRACE_EXITRC ( SDB_SDBIMP_RESOLVEARG, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBIMP_GETCOLLECTION, "getCollection" )
INT32 getCollection ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBIMP_GETCOLLECTION );
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
   rc = sdbConnect ( gpHostName, gpServiceName, lUser, lPassWord, &gConnection ) ;
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
   PD_TRACE_EXITRC ( SDB_SDBIMP_GETCOLLECTION, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_IMPORTCSV, "importCSV" )
INT32 importCSV ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_IMPORTCSV );
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
                       strField, isHeaderline,
                       autoAddField, autoCompletion,
                       linePriority,
                       stringType,
                       strDelChar.length()?strDelChar.c_str():NULL,
                       strDelField.length()?strDelField.c_str():NULL,
                       strDelRecord.length()?strDelRecord.c_str():NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize parser, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = parser->run ( total, succ ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute parser, rc = %d", rc ) ;
      goto error ;
   }
   ossPrintf ( "%d records in CSV file, %d records import"OSS_NEWLINE, total, succ ) ;
done :
   if ( parser )
      SDB_OSS_DEL parser ;
   PD_TRACE_EXITRC ( SDB_IMPORTCSV, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_IMPORTJSON, "importJson" )
INT32 importJson ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_IMPORTJSON );
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
   rc = parser->init ( gCollection, gpInputFileName,
                       linePriority, bMongoCompatible,
                       strDelRecord.length()?strDelRecord.c_str():NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize parser, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = parser->run ( total, succ ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute parser, rc = %d", rc ) ;
      goto error ;
   }
   ossPrintf ( "%d records in Json file, %d records import"OSS_NEWLINE, total, succ ) ;
done :
   if ( parser )
      SDB_OSS_DEL parser ;
   PD_TRACE_EXITRC ( SDB_IMPORTJSON, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBIMP_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBIMP_MAIN );

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
   case IMPORT_TYPE_CSV :
      rc = importCSV () ;
      break ;
   case IMPORT_TYPE_JSON :
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
   if ( lUser )
   {
      SDB_OSS_FREE ( lUser ) ;
   }
   if ( lPassWord )
   {
      SDB_OSS_FREE ( lPassWord ) ;
   }
   if ( strField )
      SDB_OSS_FREE ( strField ) ;
   PD_TRACE_EXITRC ( SDB_SDBIMP_MAIN, rc );
   return rc ;
error :
   goto done ;
}
