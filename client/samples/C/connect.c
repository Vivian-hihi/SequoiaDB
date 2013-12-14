/******************************************************************************
 *
 * Name: connect.c
 * Description: This program demostrates how to connect to SequoiaDB database.
 * Parameters:
 *              HostName: The hostname for database server
 *              ServiceName: The service name or port number for the database
 *                           service
 *              Username: The user name for database server
 *              Password: The password  for user
 * Auto Compile:
 * Linux: ./buildApp.sh connect
 * Win: buildApp.bat connect
 * Manual Compile:
 * Linux: cc connect.c common.c -o query -I../../include -L../../lib -lsdbc
 * Win:
 *    cl /Foconnect.obj /c connect.c /I..\..\include /wd4047
 *    cl /Focommon.obj /c common.c /I..\..\include /wd4047
 *    link /OUT:connect.exe /LIBPATH:..\..\lib sdbc.lib connect.obj common.obj
 *    copy ..\..\lib\sdbc.dll .
 * Run:
 * Linux: LD_LIBRARY_PATH=<path for libsdbc.so> ./connect <hostname> <servicename> \
 *        <Username> <Username>
 * Win: insert.exe <hostname> <servicename> <Username> <Username>
 *
 ******************************************************************************/
#include <stdio.h>
#include "common.h"


INT32 main ( INT32 argc, CHAR **argv )
{
   // define a connecion handle; use to connect to database
   sdbConnectionHandle connection    = 0 ;
   INT32 rc = SDB_OK ;

   // read argument
   CHAR *pHostName    = (CHAR*)argv[1] ;
   CHAR *pServiceName = (CHAR*)argv[2] ;
   CHAR *pUsr         = (CHAR*)argv[3] ;
   CHAR *pPasswd      = (CHAR*)argv[4] ;

   // verify syntax
   if ( 5 != argc )
   {
      displaySyntax ( (CHAR*)argv[0] ) ;
      exit ( 0 ) ;
   }

   // connect to database
   rc = sdbConnect ( pHostName, pServiceName, pUsr, pPasswd, &connection ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to connet to database, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   else
      printf( "connect success!\n") ;

done:
   // disconnect from database
   sdbDisconnect ( connection ) ;
   // release connection
   sdbReleaseConnection ( connection ) ;
   return 0 ;
error:
   goto done ;
}


