/*******************************************************************************

   OCO SOURCE MATERIALS

   SEQUOIADB CONFIDENTIAL (SEQUOIADB CONFIDENTIAL-RESTRICTED when combined
              with the Aggregated OCO Source Modules for this Program)

   COPYRIGHT: xxxxx (C) Copyright SequoiaDB Inc. 2012
              Licensed Materials - Program Property of SequoiaDB Inc.

   The source code for this program is not published or otherwise divested of
   its trade secrets, irrespective of what has been deposited with the Copyright
   Protection Center of China

   Source File Name = sdbexprt.cpp

   Descriptive Name = Export Utility

   When/how to use: this program may be used on binary and text-formatted
   versions of PMD component. This file contains main function for sdbexprt
   which is used to do data export

   Dependencies: N/A

   Restrictions: N/A

   Change Activity:
   defect Date        Who Description
   ====== =========== === ==============================================
          01/21/2013  TW  Initial Draft

   Last Changed =

*******************************************************************************/
#include "core.hpp"
#include "migExport.hpp"
#include "pd.hpp"
#include "ossUtil.hpp"
#include "ossMem.hpp"
#include "ossSocket.hpp"
#include "ossSignal.hpp"
#include "ossPrimitiveFileOp.hpp"
#include "ossStackDump.hpp"
#include "pdTrace.hpp"
#include "pmdTrace.hpp"

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
#define LOGPATH "sdbexport.log"

#define OPTION_HELP              "help"
#define OPTION_HOSTNAME          "hostname"
#define OPTION_SVCNAME           "svcname"
#define OPTION_USER              "user"
#define OPTION_PASSWORD          "password"
#define OPTION_EXPORTTYPE        "type"
#define OPTION_OUTPUTFILE        "output"
#define OPTION_DELCHAR           "delchar"
#define OPTION_DELFIELD          "delfield"
#define OPTION_DELRECORD         "delrecord"
#define OPTION_CSVFIELDLIST      "fields"
#define OPTION_CSVFIELDINCLUDED  "fieldincluded"
#define OPTION_CSNAME            "csname"
#define OPTION_CLNAME            "clname"
#define OPTION_FILENAME          "file"
#define OPTION_TYPE              FIELD_NAME_LTYPE

#define DEFAULT_OUTPUT_FILE      "sdbexport"
#define DEFAULT_HOSTNAME         "localhost"
#define FIELD_SEPARATOR          ","

#define ADD_PARAM_OPTIONS_BEGIN( desc )\
        desc.add_options()

#define ADD_PARAM_OPTIONS_END ;

#define COMMANDS_STRING( a, b ) (string(a) +string( b)).c_str()
#define COMMANDS_OPTIONS \
       ( OPTION_HELP, "help" )\
       ( COMMANDS_STRING(OPTION_HOSTNAME,          ",h"), boost::program_options::value<string>(), "database host name") \
       ( COMMANDS_STRING(OPTION_SVCNAME,           ",s"), boost::program_options::value<string>(), "database service name" ) \
       ( COMMANDS_STRING(OPTION_USER,              ",u"), boost::program_options::value<string>(), "databse user" ) \
       ( COMMANDS_STRING(OPTION_PASSWORD,          ",w"), boost::program_options::value<string>(), "databse password" ) \
       ( COMMANDS_STRING(OPTION_DELCHAR,           ",a"), boost::program_options::value<string>(), "string delimiter ( default: \" )( csv only )" ) \
       ( COMMANDS_STRING(OPTION_DELFIELD,          ",e"), boost::program_options::value<string>(), "field delimiter ( default: , )( csv only )" ) \
       ( COMMANDS_STRING(OPTION_DELRECORD,         ",r"), boost::program_options::value<string>(), "record delimiter ( default: '\\n' )" ) \
       ( COMMANDS_STRING(OPTION_CSNAME,            ",c"), boost::program_options::value<string>(), "collection space name" ) \
       ( COMMANDS_STRING(OPTION_CLNAME,            ",l"), boost::program_options::value<string>(), "collection name" ) \
       ( OPTION_CSVFIELDLIST,     boost::program_options::value<string>(), "field list ( separate by , )" ) \
       ( OPTION_CSVFIELDINCLUDED, boost::program_options::value<string>(), "include field names ( default: true )( csv only )" ) \
       ( OPTION_FILENAME,         boost::program_options::value<string>(), "database load file name" ) \
       ( OPTION_EXPORTTYPE,       boost::program_options::value<string>(), "type of file to load, default: csv (json,csv)" ) \

enum SDBEXPORT_TYPE
{
   EXPORT_TYPE_CSV = 0,
   EXPORT_TYPE_BIN,
   EXPORT_TYPE_JSON,
   EXPORT_TYPE_FINISH
} ;

CHAR *SDBEXPORT_TYPE_STR[] =
{
   "csv",
   "bin",
   "json"
} ;

CHAR gpHostName[OSS_MAX_HOSTNAME+1]       = {0} ;
CHAR gpServiceName[OSS_MAX_SERVICENAME+1] = {0} ;
SDBEXPORT_TYPE gExportType                = EXPORT_TYPE_CSV ;
CHAR *gpOutputFileName                    = NULL ;
string strDelChar                         = "" ;
string strDelField                        = "" ;
string strDelRecord                       = "" ;
string strFieldList                       = "" ;
string strCSName                          = "" ;
string strCLName                          = "" ;
CHAR *lUser                               = NULL ;
CHAR *lPassWord                           = NULL ;
BOOLEAN bFieldIncluded                    = TRUE ;
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
PD_TRACE_DECLARE_FUNCTION ( SDB_MIGTRAPHNDL1, "migTrapHandler" )
void migTrapHandler ( OSS_HANDPARMS )
{
   PD_TRACE_ENTRY ( SDB_MIGTRAPHNDL1 );
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
   PD_TRACE_EXIT ( SDB_MIGTRAPHNDL1 );
   return ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_MIGSSH1, "migSetupSignalHandler" )
INT32 migSetupSignalHandler()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_MIGSSH1 );
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
   PD_TRACE_EXITRC ( SDB_MIGSSH1, rc );
   return rc ;
error :
   goto done ;
}
#endif


// check whether the file name includes suffix
BOOLEAN suffixIncluded ( const CHAR *pFileName, const CHAR *pSuffix )
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
}

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

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBEXP_RESOLVEARG, "resolveArgument" )
INT32 resolveArgument ( po::options_description &desc, INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBEXP_RESOLVEARG );
   po::variables_map vm ;
   const CHAR *pFileName = NULL ;
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

   // export type is optional, default is csv
   if ( vm.count ( OPTION_EXPORTTYPE ) )
   {
      for ( UINT32 i = 0; i < EXPORT_TYPE_FINISH; ++i )
      {
         if ( ossStrncasecmp ( vm[OPTION_EXPORTTYPE].as<string>().c_str(),
                               SDBEXPORT_TYPE_STR[i],
                               ossStrlen ( SDBEXPORT_TYPE_STR[i] ) ) == 0 )
         {
            gExportType = (SDBEXPORT_TYPE)i ;
            break ;
         }
      }
   }

   // output file is optional, default is "sdbexport.<type>"
   if ( vm.count ( OPTION_FILENAME ) )
   {
      pFileName = vm[OPTION_FILENAME].as<string>().c_str() ;
      INT32 bufSize = ossStrlen ( pFileName ) + 1 ;
      gpOutputFileName = (CHAR*)SDB_OSS_MALLOC ( bufSize ) ;
      if ( !gpOutputFileName )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory for %d bytes",
                  bufSize ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      ossStrncpy ( gpOutputFileName, pFileName, bufSize ) ;
   }
   else
   {
      PD_LOG ( PDERROR, "file name must input" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

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
   // csv field list
   if ( vm.count ( OPTION_CSVFIELDLIST ) )
   {
      strFieldList = vm[OPTION_CSVFIELDLIST].as<string>() ;
   }
   // whether include csv field name
   if ( vm.count ( OPTION_CSVFIELDINCLUDED ) )
   {
      ossStrToBoolean ( vm[OPTION_CSVFIELDINCLUDED].as<string>().c_str(),
                        &bFieldIncluded ) ;
   }

   // collection space name
   if ( vm.count ( OPTION_CSNAME ) )
   {
      strCSName = vm[OPTION_CSNAME].as<string>() ;
   }
   // collection name
   if ( vm.count ( OPTION_CLNAME ) )
   {
      strCLName = vm[OPTION_CLNAME].as<string>() ;
   }
done :
   PD_TRACE_EXITRC ( SDB_SDBEXP_RESOLVEARG, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBEXP_GETCOLLECTION, "getCollection" )
INT32 getCollection ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBEXP_GETCOLLECTION );
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
   rc = sdbConnect ( gpHostName, gpServiceName, lUser, lPassWord,
                     &gConnection ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to connect to database %s:%s, rc = %d"OSS_NEWLINE,
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
   PD_TRACE_EXITRC ( SDB_SDBEXP_GETCOLLECTION, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_EXPORTCSV, "exportCSV" )
INT32 exportCSV ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_EXPORTCSV );
   migCSVExtractor *extractor = NULL ;
   CHAR *pFieldList = NULL ;
   CHAR *pCurPos = NULL ;
   CHAR *pSepPos = NULL ;
   vector<string> fieldList ;
   // make sure fields are specified
   if ( !strFieldList.length() )
   {
      ossPrintf ( "Field list is required for CSV file, \
fields are separated by comma ( ',' )"OSS_NEWLINE ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   // string duplicate, free by end of function
   pFieldList = ossStrdup ( strFieldList.c_str() ) ;
   if ( !pFieldList )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory to duplicate field list" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   pCurPos = pFieldList ;
   while ( ( pSepPos = ossStrstr ( pCurPos, FIELD_SEPARATOR )) != NULL  )
   {
      *pSepPos = '\0' ;
      string str ( pCurPos ) ;
      fieldList.push_back ( trim ( str ) ) ;
      pCurPos = pSepPos + 1 ;
   }
   if ( pCurPos && *pCurPos )
   {
      string str ( pCurPos ) ;
      fieldList.push_back ( trim ( str ) ) ;
   }
   // create new extractor object, free by end of function
   extractor = SDB_OSS_NEW migCSVExtractor ( fieldList, bFieldIncluded ) ;
   if ( !extractor )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for extractor" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   // initialize
   rc = extractor->init ( gCollection, gpOutputFileName,
                          strDelChar.length()?strDelChar.c_str():NULL,
                          strDelField.length()?strDelField.c_str():NULL,
                          strDelRecord.length()?strDelRecord.c_str():NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize extractor, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = extractor->run () ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute extractor, rc = %d", rc ) ;
      goto error ;
   }
done :
   if ( pFieldList )
      SDB_OSS_FREE ( pFieldList ) ;
   if ( extractor )
      SDB_OSS_DEL extractor ;
   PD_TRACE_EXITRC ( SDB_EXPORTCSV, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_EXPORTJSON, "exportJSON" )
INT32 exportJSON ()
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_EXPORTJSON );
   migJSONExtractor *extractor = NULL ;
   CHAR *pFieldList = NULL ;
   CHAR *pCurPos = NULL ;
   CHAR *pSepPos = NULL ;
   vector<string> fieldList ;
   // make sure fields are specified
   if ( strFieldList.length() )
   {
      // string duplicate, free by end of function
      pFieldList = ossStrdup ( strFieldList.c_str() ) ;
      if ( !pFieldList )
      {
         PD_LOG ( PDERROR, "Failed to allocate memory to duplicate field list" ) ;
         rc = SDB_OOM ;
         goto error ;
      }
      pCurPos = pFieldList ;
      while ( ( pSepPos = ossStrstr ( pCurPos, FIELD_SEPARATOR )) != NULL  )
      {
         *pSepPos = '\0' ;
         string str ( pCurPos ) ;
         fieldList.push_back ( trim ( str ) ) ;
         pCurPos = pSepPos + 1 ;
      }
      if ( pCurPos && *pCurPos )
      {
         string str ( pCurPos ) ;
         fieldList.push_back ( trim ( str ) ) ;
      }
   }
   // create new extractor object, free by end of function
   extractor = SDB_OSS_NEW migJSONExtractor ( fieldList ) ;
   if ( !extractor )
   {
      PD_LOG ( PDERROR, "Failed to allocate memory for extractor" ) ;
      rc = SDB_OOM ;
      goto error ;
   }
   // initialize
   rc = extractor->init ( gCollection, gpOutputFileName,
                          strDelRecord.length()?strDelRecord.c_str():NULL ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize extractor, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = extractor->run () ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute extractor, rc = %d", rc ) ;
      goto error ;
   }
done :
   if ( pFieldList )
      SDB_OSS_FREE ( pFieldList ) ;
   if ( extractor )
      SDB_OSS_DEL extractor ;
   PD_TRACE_EXITRC ( SDB_EXPORTJSON, rc );
   return rc ;
error :
   goto done ;
}

PD_TRACE_DECLARE_FUNCTION ( SDB_SDBEXP_MAIN, "main" )
INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   PD_TRACE_ENTRY ( SDB_SDBEXP_MAIN );

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
   switch ( gExportType )
   {
   case EXPORT_TYPE_CSV :
      rc = exportCSV () ;
      break ;
   case EXPORT_TYPE_BIN :
      break ;
   case EXPORT_TYPE_JSON :
      rc = exportJSON () ;
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
         ossPrintf ( "Export Failed"OSS_NEWLINE ) ;
      }
   }
   else
   {
      ossPrintf ( "Export Successfully"OSS_NEWLINE ) ;
   }
   if ( rc != SDB_PMD_HELP_ONLY )
   {
      ossPrintf ( "Detail in log path: %s"OSS_NEWLINE, _pdDiagLogPath ) ;
   }
   sdbDisconnect ( gConnection ) ;
   if ( gpOutputFileName )
      SDB_OSS_FREE ( gpOutputFileName ) ;
   if ( lUser )
   {
      SDB_OSS_FREE ( lUser ) ;
   }
   if ( lPassWord )
   {
      SDB_OSS_FREE ( lPassWord ) ;
   }
   PD_TRACE_EXITRC ( SDB_SDBEXP_MAIN, rc );
   return rc ;
error :
   goto done ;
}
