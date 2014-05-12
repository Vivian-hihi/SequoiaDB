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
#include "pdTrace.hpp"
#include "pmdTrace.hpp"
#include "msgDef.h"
#include "utilSdb.hpp"
#include <iostream>

#define LOGPATH "sdbimport.log"

#define OPTION_DELCHAR           "delchar"
#define OPTION_DELFIELD          "delfield"
#define OPTION_DELRECORD         "delrecord"
#define OPTION_FILENAME          "file"
#define OPTION_EXTRA             "extra"
#define OPTION_SPARSE            "sparse"
#define OPTION_LINEPRIORITY      "linepriority"
#define OPTION_STRINGTYPE        "stringtype"
#define OPTION_FIELD             FIELD_NAME_FIELDS
#define OPTION_HEADERLINE        FIELD_NAME_HEADERLINE
#define OPTION_COLLECTSPACE      "csname"
#define OPTION_COLLECTION        "clname"
#define OPTION_TYPE              FIELD_NAME_LTYPE

#define DEFAULT_HOSTNAME         "localhost"
#define DEFAULT_SVCNAME          "11810"

utilSdbTemplet utilSdbObj ;
sdbConnectionHandle gConnection ;
sdbCSHandle         gCollectionSpace ;
sdbCollectionHandle gCollection ;

const CHAR *SDBIMPORT_TYPE_STR[] =
{
   "csv",
   "json"
} ;

INT32 on_init( void *pData )
{
   INT32 rc = SDB_OK ;
done:
   return rc ;
error:
   goto done ;
}

INT32 on_preparation( void *pData )
{
   INT32 rc = SDB_OK ;
   CHAR *pHostname = NULL ;
   CHAR *pSvcname  = NULL ;
   CHAR *pUser     = NULL ;
   CHAR *pPassword = NULL ;
   CHAR *pCsname   = NULL ;
   CHAR *pClname   = NULL ;

   utilSdbObj.getArgString( OPTION_HOSTNAME, &pHostname ) ;
   utilSdbObj.getArgString( OPTION_SVCNAME,  &pSvcname ) ;
   utilSdbObj.getArgString( OPTION_USER,     &pUser ) ;
   utilSdbObj.getArgString( OPTION_PASSWORD, &pPassword ) ;
   utilSdbObj.getArgString( OPTION_COLLECTSPACE, &pCsname ) ;
   utilSdbObj.getArgString( OPTION_COLLECTION,   &pClname ) ;
   // connection is established
   rc = sdbConnect ( pHostname, pSvcname, pUser, pPassword, &gConnection ) ;
   if ( rc )
   {
      ossPrintf ( "Failed to connect to database %s:%s, rc = %d",
                  pHostname, pSvcname, rc ) ;
      goto error ;
   }

   // get collection space
   rc = sdbGetCollectionSpace ( gConnection, pCsname,
                                &gCollectionSpace ) ;
   if ( SDB_DMS_CS_NOTEXIST == rc )
   {
      ossPrintf ( "Collection space %s does not exist"OSS_NEWLINE,
                  pCsname ) ;
      goto error ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection space %s, rc = %d",
               pCsname, rc ) ;
      goto error ;
   }

   // get collection
   rc = sdbGetCollection1 ( gCollectionSpace, pClname,
                            &gCollection ) ;
   if ( SDB_DMS_NOTEXIST == rc )
   {
      ossPrintf ( "Collection %s does not exist"OSS_NEWLINE,
                  pClname ) ;
      goto error ;
   }
   else if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to get collection %s, rc = %d",
               pClname, rc ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}

INT32 importCSV ()
{
   INT32 rc = SDB_OK ;
   INT32 total = 0, succ = 0 ;
   BOOLEAN isHeaderline   = TRUE ;
   BOOLEAN autoAddField   = TRUE ;
   BOOLEAN autoCompletion = TRUE ;
   BOOLEAN linePriority   = TRUE ;
   CHAR *pFile      = NULL ;
   CHAR *pFields    = NULL ;
   CHAR *pDelChar   = NULL ;
   CHAR *pDelField  = NULL ;
   CHAR *pDelRecord = NULL ;
   migCSVParser parser ;

   utilSdbObj.getArgString( OPTION_FILENAME,  &pFile ) ;
   utilSdbObj.getArgString( OPTION_FIELD,     &pFields ) ;
   utilSdbObj.getArgString( OPTION_DELCHAR,   &pDelChar ) ;
   utilSdbObj.getArgString( OPTION_DELFIELD,  &pDelField ) ;
   utilSdbObj.getArgString( OPTION_DELRECORD, &pDelRecord ) ;

   utilSdbObj.getArgBool( OPTION_HEADERLINE,   &isHeaderline ) ;
   utilSdbObj.getArgBool( OPTION_SPARSE,       &autoAddField ) ;
   utilSdbObj.getArgBool( OPTION_EXTRA,        &autoCompletion ) ;
   utilSdbObj.getArgBool( OPTION_LINEPRIORITY, &linePriority ) ;

   if ( !isHeaderline && !pFields )
   {
      ossPrintf ( "if not read first line,than must input fields" ) ;
      PD_LOG ( PDERROR, "if not read first line,than must input fields" ) ;
      rc = SDB_INVALIDARG ;
      goto error ;
   }

   // initialize
   rc = parser.init ( gCollection, pFile,
                      pFields, isHeaderline,
                      autoAddField, autoCompletion,
                      linePriority,
                      FALSE,
                      pDelChar,
                      pDelField,
                      pDelRecord ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize parser, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = parser.run ( total, succ ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute parser, rc = %d", rc ) ;
      goto error ;
   }
   ossPrintf ( "%d records in CSV file, %d records import"OSS_NEWLINE, total, succ ) ;
done :
   return rc ;
error :
   goto done ;
}

INT32 importJson ()
{
   INT32 rc = SDB_OK ;
   INT32 total = 0, succ = 0 ;
   BOOLEAN linePriority   = TRUE ;
   CHAR *pFile      = NULL ;
   CHAR *pDelRecord = NULL ;
   migJSONParser parser ;

   utilSdbObj.getArgString( OPTION_FILENAME,  &pFile ) ;
   utilSdbObj.getArgString( OPTION_DELRECORD, &pDelRecord ) ;
   utilSdbObj.getArgBool( OPTION_LINEPRIORITY, &linePriority ) ;

   // initialize
   rc = parser.init ( gCollection, pFile,
                      linePriority, FALSE,
                      pDelRecord ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to initialize parser, rc = %d", rc ) ;
      goto error ;
   }
   // run it
   rc = parser.run ( total, succ ) ;
   if ( rc )
   {
      PD_LOG ( PDERROR, "Failed to execute parser, rc = %d", rc ) ;
      goto error ;
   }
   ossPrintf ( "%d records in Json file, %d records import"OSS_NEWLINE, total, succ ) ;
done :
   return rc ;
error :
   goto done ;
}

INT32 on_main( void *pData )
{
   INT32 rc = SDB_OK ;
   INT32 type = 0 ;

   utilSdbObj.getArgSwitch( OPTION_TYPE,  &type ) ;

   switch( type )
   {
   case 0:
      rc = importCSV () ;
      break ;
   case 1:
      rc = importJson () ;
      break ;
   default :
      rc = SDB_INVALIDARG ;
      PD_LOG ( PDERROR, "Invalid type" ) ;
      goto error ;
   }
done:
   return rc ;
error:
   goto done ;
}
INT32 on_end( void *pData )
{
   INT32 rc = SDB_OK ;
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
done:
   return rc ;
error:
   goto done ;
}

INT32 main ( INT32 argc, CHAR **argv )
{
   INT32 rc = SDB_OK ;
   util_sdb_settings setting ;

   setting.on_init = on_init ;
   setting.on_preparation = on_preparation ;
   setting.on_main = on_main ;
   setting.on_end = on_end ;

   sdbEnablePD( LOGPATH ) ;
   setPDLevel( PDINFO ) ;

   APPENDARGSTRING( utilSdbObj, OPTION_HOSTNAME,     OPTION_HOSTNAME ",s",     "database host name ( default: localhost )",                                                        FALSE, OSS_MAX_HOSTNAME,    DEFAULT_HOSTNAME ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_SVCNAME,      OPTION_SVCNAME ",p",      "database service name ( default: 11810 )",                                                     FALSE, OSS_MAX_SERVICENAME, DEFAULT_SVCNAME ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_USER,         OPTION_USER ",u",         "databse user",                                                              FALSE, -1,                  "\0" ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_PASSWORD,     OPTION_PASSWORD ",w",     "databse password",                                                          FALSE, -1,                  "\0" ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_DELCHAR,      OPTION_DELCHAR ",a",      "string delimiter ( default: \" )( csv only )",                              FALSE, 4,                  "\"" ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_DELFIELD,     OPTION_DELFIELD ",e",     "field delimiter ( default: , )( csv only )",                                FALSE, 4,                   ","  ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_DELRECORD,    OPTION_DELRECORD ",r",    "record delimiter ( default: '\\n' )",                                       FALSE, 4,                   "\n" ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_COLLECTSPACE, OPTION_COLLECTSPACE ",c", "collection space name",                                                     TRUE,  -1,                  NULL ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_COLLECTION,   OPTION_COLLECTION ",l",   "collection name",                                                           TRUE,  -1,                  NULL ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_FILENAME,     OPTION_FILENAME,          "database load file name",                                                   TRUE,  -1,                  NULL ) ;
   APPENDARGSWITCH( utilSdbObj, OPTION_TYPE,         OPTION_TYPE,              "type of file to load, default: json (json,csv)",                            FALSE, SDBIMPORT_TYPE_STR,  2,   "json" ) ;
   APPENDARGSTRING( utilSdbObj, OPTION_FIELD,        OPTION_FIELD,             "comma separated list of field names e.g. --fields name,age ( csv only )",   FALSE, -1,                  NULL ) ;
   APPENDARGBOOL  ( utilSdbObj, OPTION_HEADERLINE,   OPTION_HEADERLINE,        "first line in input file is a header, default: false ( csv only )",         FALSE, FALSE ) ;
   APPENDARGBOOL  ( utilSdbObj, OPTION_SPARSE,       OPTION_SPARSE,            "auto add fields, default: true ( csv only )",                               FALSE, TRUE  ) ;
   APPENDARGBOOL  ( utilSdbObj, OPTION_EXTRA,        OPTION_EXTRA,             "auto add value, default: false ( csv only )",                               FALSE, FALSE ) ;
   APPENDARGBOOL  ( utilSdbObj, OPTION_LINEPRIORITY, OPTION_LINEPRIORITY,      "reverse the priority for record and character delimiter, default: true",    FALSE, TRUE  ) ;

   rc = utilSdbObj.init( setting, NULL ) ;
   if ( rc )
   {
      goto error ;
   }

   rc = utilSdbObj.run( argc, argv ) ;
   if ( rc )
   {
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
      ossPrintf ( "Detail in log path: %s"OSS_NEWLINE, getDialogName() ) ;
      PD_LOG ( PDEVENT, "Import Completed" ) ;
   }
   return rc ;
error :
   goto done ;
}
