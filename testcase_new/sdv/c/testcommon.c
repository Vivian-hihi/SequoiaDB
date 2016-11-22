#include <stdlib.h>
#include <stdio.h>
#include "testcommon.h"

char HOSTNAME[100] ;
char SVCNAME[100] ;
char CHANGEDPREFIX[100] ;
char* confFile = "driver.conf" ;

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

void getConf()
{
    FILE *fp ;
    fp = fopen(confFile,"rt") ;
    if(fp == NULL)
        printf("Cannot open file driver.conf") ;
    char str[100] ;
    while( fscanf(fp,"%s",str) != EOF )
    {
        char* token = strstr(str,"=") ;
        token[0] = '\0' ;
        if(strcmp(str,"HOSTNAME") == 0)
        {
            strcpy(HOSTNAME,token+1) ;
            continue ;
        }
        if(strcmp(str,"SVCNAME") == 0)
        {
            strcpy(SVCNAME,token+1) ;
            continue ;
        }
        if(strcmp(str,"CHANGEDPREFIX") == 0)
        {
            strcpy(CHANGEDPREFIX,token+1) ;
            continue ;
        }

    }
    fclose(fp) ;
    printf("HostName: %s\n",HOSTNAME) ;
    printf("SvcName: %s\n",SVCNAME) ;
    printf("CHANGEDPREFIX: %s\n",CHANGEDPREFIX) ;
}
