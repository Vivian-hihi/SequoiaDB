/******************************************************************************
 *
 * Name: query.c
 * Description: This program demostrates how to connect to query from database.
 * Parameters:
 *              HostName: The hostname for database server
 *              ServiceName: The service name or port number for the database
 *                           service
 *              Username: The user name for database server
 *              Password: The password  for user
 * Auto Compile:
 * Linux: ./buildApp.sh query
 * Win: buildApp.bat query
 * Manual Compile:
 * Linux: cc query.c common.c -o query -I../../include -L../../lib -lsdbc
 * Win:
 *    cl /Foquery.obj /c query.c /I..\..\include /wd4047
 *    cl /Focommon.obj /c common.c /I..\..\include /wd4047
 *    link /OUT:query.exe /LIBPATH:..\..\lib sdbc.lib query.obj common.obj
 *    copy ..\..\lib\sdbc.dll .
 * Run:
 * Linux: LD_LIBRARY_PATH=<path for libsdbc.so> ./query <hostname> <servicename> \
 *        <Username> <Username>
 * Win: query.exe <hostname> <servicename> <Username> <Username>
 *
 ******************************************************************************/
#include <stdio.h>
#include "client.h"

#define NUM_RECORD 5

#define COLLECTION_SPACE_NAME "foo"
#define COLLECTION_NAME       "bar"
#define COLLECTION_FULL_NAME  "foo.bar"

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
   INT32 rc    = SDB_OK ;
   INT32 count = 0 ;
   bson obj ;
   bson rule ;
   bson objList [ NUM_RECORD ] ;

   // verify syntax
   if ( 5 != argc )
   {
      displaySyntax ( (CHAR*)argv[0] ) ;
      exit ( 0 ) ;
   }
   // read argument
   pHostName    = (CHAR*)argv[1] ;
   pServiceName = (CHAR*)argv[2] ;
   pUsr         = (CHAR*)argv[3] ;
   pPasswd      = (CHAR*)argv[4] ;

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
   waiting( 1 ) ;
   // create collection in a specified colletion space.
   // Here,we build it up in the new collection.
   rc = sdbCreateCollection ( collectionspace, COLLECTION_NAME, &collection ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to create collection, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }
   // recommned to wait for a few seconds in cluster environment
   waiting( 1 ) ;

   // create record list using objList
   createRecordList ( &objList[0], NUM_RECORD ) ;
   // insert obj and free memory that allocated by createRecordList
   for ( count = 0; count < NUM_RECORD; count++ )
   {
      // all the contents of inserted record are the same except _id
      rc = sdbInsert ( collection, &objList[count] ) ;
      if ( rc )
      {
         printf ( "Failed to insert record, rc = %d" OSS_NEWLINE, rc ) ;
      }
      bson_destroy ( &objList[count] ) ;
   }

   // query all the record in this collection
   // and return the result by the cursor handle
   rc = sdbQuery(collection, NULL, NULL, NULL, NULL, 0, -1, &cursor ) ;
   if( rc!=SDB_OK )
   {
      printf("Failed to query, rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }

   // get all the qureied records
   bson_init(&obj);
   while( !( rc=sdbNext( cursor, &obj ) ) )
   {
      if( rc!=SDB_OK )
      {
         printf("Failed to get the current record,\
                 rc = %d" OSS_NEWLINE, rc ) ;
         goto error ;
      }
      bson_print( &obj ) ;
      bson_destroy(&obj) ;
      bson_init(&obj);
   }
   bson_destroy(&obj) ;
   if( rc==SDB_DMS_EOC )
   {
      printf("All the record had been list." OSS_NEWLINE ) ;
   }
   else if( rc!=SDB_OK )
   {
      printf("Failed to get the current record,\
              rc = %d" OSS_NEWLINE, rc ) ;
      goto error ;
   }

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
   sdbReleaseCursor ( cursor ) ;
   sdbReleaseCollection ( collection ) ;
   sdbReleaseCS ( collectionspace ) ;
   sdbReleaseConnection ( connection ) ;
   return 0;
error:
   goto done ;
}
