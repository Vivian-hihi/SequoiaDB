#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "testcommon.hpp"

using namespace std ;
using testing::internal::String ;
using testing::internal::g_argvs ;
extern vector<String> g_argvs ;

// default value
char HOSTNAME[100] = "localhost" ;
char SVCNAME[100] = "11810" ;
char CHANGEDPREFIX[100] = "sdv_c_test" ;

INT32 createCollection( sdbCollectionHandle *cl,
                        CHAR *csName,
                        CHAR *clName )
{
   INT32 rc = SDB_OK;
   sdbConnectionHandle db = 0;
   sdbCSHandle cs = 0;
   bson clOptions;   

   // connect to sdb
   getConf() ;
   rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db );
   if ( rc )
   {
      printf( "Failed to connect database, hostname=%s, coord= %s\n",
              HOSTNAME, SVCNAME );     
      goto error ;
   }
   
   // create cs   
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
    printf( "Print command args:\n" ) ;
    for( int i = 0;i < g_argvs.size();i++ )
      printf( "%s ",g_argvs[i].c_str() ) ;
    printf( "\n" ) ;
    
    for( int i = 0;i < g_argvs.size();i++ )
    {
      String para = g_argvs[i] ;
      if( para == "--hostname" || para == "-n" )
         strcpy( HOSTNAME,g_argvs[i+1].c_str() ) ;
      else if( para == "--svcname" || para == "-s" )
         strcpy( SVCNAME,g_argvs[i+1].c_str() ) ;
      else if( para == "--changedprefix" || para == "-c" )
         strcpy( CHANGEDPREFIX,g_argvs[i+1].c_str() ) ;
    }
    
    printf("HostName: %s\n",HOSTNAME) ;
    printf("SvcName: %s\n",SVCNAME) ;
    printf("CHANGEDPREFIX: %s\n",CHANGEDPREFIX) ;
    printf("\n") ;
}

void getUniqueName(const char* modName,char name[])
{
   pid_t pid = getpid() ;
   sprintf( name,"%s_%d_%s",CHANGEDPREFIX,pid,modName ) ;
}
