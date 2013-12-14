/******************************************************************************
 *
 * Name: index.c
 * Description: This program demostrates how to connect to create index.
 * Parameters:
 *              HostName: The hostname for database server
 *              ServiceName: The service name or port number for the database
 *                           service
 *              Username: The user name for database server
 *              Password: The password  for user
 * Auto Compile:
 * Linux: ./buildApp.sh index
 * Win: buildApp.bat index
 * Manual Compile:
 * Linux: cc index.c common.c -o query -I../../include -L../../lib -lsdbc
 * Win:
 *    cl /Foindex.obj /c index.c /I..\..\include /wd4047
 *    cl /Focommon.obj /c common.c /I..\..\include /wd4047
 *    link /OUT:index.exe /LIBPATH:..\..\lib sdbc.lib index.obj common.obj
 *    copy ..\..\lib\sdbc.dll .
 * Run:
 * Linux: LD_LIBRARY_PATH=<path for libsdbc.so> ./insert <hostname> <servicename> \
 *        <Username> <Username>
 * Win: insert.exe <hostname> <servicename> <Username> <Username>
 *
 ******************************************************************************/
#include <stdio.h>
#include "common.h"

#define COLLECTION_SPACE_NAME "foo"
#define COLLECTION_NAME       "bar"
#define INDEX_NAME            "index"

INT32 main ( INT32 argc, CHAR **argv )
{
   // initialize local variables
   CHAR *pHostName                   = NULL ;
   CHAR *pServiceName                = NULL ;
   CHAR *pUsr                        = NULL ;
   CHAR *pPasswd                     = NULL ;
   // define a connetion handle; use to connect to database
   sdbConnectionHandle connection    = 0 ;
   // define a collection space handle
   sdbCSHandle collectionspace       = 0 ;
   // define a collection handle
   sdbCollectionHandle collection    = 0 ;
   // define a cursor handle for query
   sdbCursorHandle cursor            = 0 ;

   // define local variables
   // initialize them before use
   bson obj ;
   INT32 rc = SDB_OK ;

   // read argument
   pHostName    = (CHAR*)argv[1] ;
   pServiceName = (CHAR*)argv[2] ;
   pUsr         = (CHAR*)argv[3] ;
   pPasswd      = (CHAR*)argv[4] ;

   // verify syntax
   if ( 5 != argc )
   {
      displaySyntax ( (CHAR*)argv[0] ) ;
      exit ( 1 ) ;
   }

   // connect to database
   rc = sdbConnect ( pHostName, pServiceName, pUsr, pPasswd, &connection ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to connet to database, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }

   // create collection space
   rc = sdbCreateCollectionSpace ( connection, COLLECTION_SPACE_NAME,
                                   SDB_PAGESIZE_4K, &collectionspace ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to create collection space, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   // recommned to wait for a few seconds in cluster environment
   waiting ( 1 ) ;

   // create collection in a specified colletion space.
   // Here,we build it up in the new collection.
   rc = sdbCreateCollection ( collectionspace, COLLECTION_NAME, &collection ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to create collection, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   // recommned to wait for a few seconds in cluster environment
   waiting ( 1 ) ;

   // create index
   bson_init( &obj ) ;
   // build a bson for index definition
   bson_append_int( &obj, "name", 1 ) ;
   bson_append_int( &obj, "age", -1 ) ;
   bson_finish( &obj ) ;
   printf("The index to build is: ") ;
   bson_print ( &obj ) ;
   rc = sdbCreateIndex ( collection, &obj, INDEX_NAME, FALSE, FALSE ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to create index, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   bson_destroy ( &obj ) ;
   printf("Suceess to build index!" OSS_NEWLINE ) ;
   // drop the specified collection
   rc = sdbDropCollection( collectionspace,COLLECTION_NAME ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to drop the specified collection,\
              rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   // drop the specified collection space
   rc = sdbDropCollectionSpace( connection,COLLECTION_SPACE_NAME ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to drop the specified collection,\
              rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
done:
   // disconnect the connection
   sdbDisconnect ( connection ) ;
   // release the local variables
   sdbReleaseCollection ( collection ) ;
   sdbReleaseCS ( collectionspace ) ;
   sdbReleaseConnection ( connection ) ;
   return rc ;
error:
   goto done ;
}

