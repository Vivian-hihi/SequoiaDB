#include <stdlib.h>
#include <stdio.h>
#include "testcommon.h"

INT32 createCollection( sdbCollectionHandle *cl,
                        CHAR *csName,
                        CHAR *clName )
{
   INT32 rc = SDB_OK;

   // connect to sdb
   sdbConnectionHandle db = 0;
   rc = sdbConnect( HOST, SERVER, "", "", &db );
   if ( rc )
   {
      printf( "Failed to connect database, hostname=%s, coord= %s\n",
              HOST, SERVER );     
      goto error ;
   }
   
   // create cs
   sdbCSHandle cs = 0;   
   rc = sdbCreateCollectionSpace( db, csName, 65536, &cs );
   if( SDB_DMS_CS_EXIST == rc )
   {
      rc = sdbGetCollectionSpace( db, csName, &cs );
   }
   if( rc )
   {
      printf( "Failed to get cs %s\n", csName );
      goto error ;
   }
   
   // create cl
   bson clOptions;
   bson_init( &clOptions );
   bson_append_int( &clOptions, "ReplSize", 0 );
   bson_finish( &clOptions );
   
   rc = sdbDropCollection( cs, clName );
   if( SDB_DMS_NOTEXIST != rc && SDB_OK != rc )
   {
      printf( "Failed to drop cl %s.%s in ready\n", csName, clName );
   }

   rc = sdbCreateCollection1( cs, clName, &clOptions, cl );
   if( rc )
   {
      printf( "Failed to create cl %s.%s\n", csName, clName );
      goto error ;
   }
   
   bson_destroy( &clOptions );
   
done:   
   return rc;
error:
   goto done;  
}

BOOLEAN isStandalone( sdbConnectionHandle db )
{
   BOOLEAN isStdalone = FALSE;
   INT32 rc = SDB_OK;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;

   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_RTN_COORD_ONLY == rc )
   {
      isStdalone = TRUE;
   }
   sdbReleaseCursor(cursor) ;
   return isStdalone;
}
