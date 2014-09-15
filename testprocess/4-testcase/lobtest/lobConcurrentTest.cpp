/*******************************************************************************
*@Description : All LOB Foundamental Function Testcase.Description of testcase
*               in directory--->
*               [ trunk/testprocess/1-process/大对象测试/Testing_LOB.xlsx ]
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtest/gtest.h>
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include "testcommon.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
/*******************************************************************************
*@Description : Test large object concurrent Open LOB.
*               { TestCase Number : LOB.CONCURRENT_TEST.004 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( ConcurrentTest, lobConcurrentOpen )
{
   const UINT64 lobSize = 1024*1024*16 ;
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   CHAR clName[70] = { 0 } ;
   bson_oid_t oid, oid1;
   bson obj ;
   pthread_t wlobTid ;
   struct Lob wlobarg ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to run create CL, rc = " << rc  ;
   // Concurrent Open different LOB [different OID]
   UINT32 i ;
   for( i = 0 ; i < 5 ; ++i )
   {
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;  // Print the oid
      wlobarg.CL = cl ;
      wlobarg.OID = oid ;
      rc = pthread_create( &wlobTid, NULL, tOpenLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                                  << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
      rc = sdbRemoveLob( cl, &oid ) ;
   }
   printf( "*=<Success Open different LOB>=*\n" ) ;
   // Concurrent Open same LOB[Same oid]
   bson_oid_gen( &oid1 ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid1 ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;  // Print the oid
   wlobarg.CL = cl ;
   wlobarg.OID = oid1 ;
   wlobarg.inspect = false ;
   for( i = 0 ; i < 5 ; ++i )
   {
      bson_print( &obj ) ;  // Print the oid
      rc = pthread_create( &wlobTid, NULL, tOpenLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                                  << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
      wlobarg.inspect = true ;
      rc = sdbRemoveLob( cl, &oid ) ;
   }
   printf( "*=<Success Open same LOB in Once>=*\n" ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object concurrent Write different LOB.
*               Test large object concurrent Remomve different LOB.
*               { TestCase Number : LOB.CONCURRENT_TEST.002 }
*               { TestCase Number : LOB.CONCURRENT_TEST.003 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( ConcurrentTest, lobConcurrentWriteRemove )
{
   const UINT64 lobSize = 1024*1024*16 ;
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR *readBuf = NULL ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 putLob = 10 ;
   bson_oid_t oid, OIDS[putLob] ;
   bson obj ;
   pthread_t wlobTid, rlobTid;
   struct Lob wlobarg ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to run create CL, rc = " << rc  ;
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Write different LOB by using diffrent data
   UINT32 i ;
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      // Write Large Object
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || (putLob-1) == i )
         bson_print( &obj ) ;  // Print the oid
      OIDS[i] = oid ;
      wlobarg.CL = cl ;
      wlobarg.OID = oid ;
      wlobarg.Buffer = lobWriteBuf ;
      wlobarg.inspect = false ;
      wlobarg.size = 0 ;
      rc = pthread_create( &wlobTid, NULL, tWriteLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                               << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
   }
   printf( "*=<Success to Concurrent Write %d LOB>=*\n", putLob ) ;
   // Remove LOB
   for( i = 0 ; i < putLob ; ++i )
   {
      wlobarg.CL = cl ;
      wlobarg.OID = OIDS[i] ;
      wlobarg.inspect = false ;
      tGetLobSize( (void *)&wlobarg ) ;
      ASSERT_TRUE( wlobarg.size == strlen(lobWriteBuf) ) << "Wrong Write Lob,"
                                                         << " Size = "
                                                         << wlobarg.size ;
      // Remove different oid LOB
      rc = pthread_create( &wlobTid, NULL, tRemoveLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                               << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
      // Remove same oid LOB
      wlobarg.inspect = true ;
      rc = pthread_create( &wlobTid, NULL, tRemoveLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                               << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
   }
   printf( "*=<Success to Concurrent Remove %d LOB>=*\n", putLob ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
}

/*******************************************************************************
*@Description : 1.Test large object concurrent Read different LOB.
*               { TestCase Number : LOB.CONCURRENT_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( ConcurrentTest, lobConcurrentRead )
{
   const UINT64 lobSize = 1024*1024*16 ;
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR *readBuf = NULL ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 putLob = 10 ;
   bson_oid_t oid, OIDS[putLob] ;
   bson obj ;
   pthread_t wlobTid, rlobTid;
   struct Lob wlobarg ;
   struct Lob rlobarg ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to run create CL, rc = " << rc  ;
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Write different LOB by using diffrent data
   UINT32 i ;
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      // Write Large Object
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || (putLob-1) == i )
         bson_print( &obj ) ;  // Print the oid
      OIDS[i] = oid ;
      wlobarg.CL = cl ;
      wlobarg.OID = oid ;
      wlobarg.Buffer = lobWriteBuf ;
      wlobarg.size = 0 ;
      rc = pthread_create( &wlobTid, NULL, tWriteLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                               << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
   }
   printf( "*=<Success to Put %d LOB>=*\n", putLob ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   if( NULL == ( lobReadBuf = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Concurrent Read Lob different OID and same OID
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobReadBuf, 0, lobSize ) ;
      rlobarg.CL = cl ;
      rlobarg.OID = OIDS[i] ;
      rlobarg.Buffer = lobReadBuf ;
      rlobarg.size = 0 ;
      tGetLobSize( (void *)&rlobarg ) ;
      rc = pthread_create( &rlobTid, NULL, tReadLob, (void *)&rlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                               << " rc =" << rc ;
      pthread_join( rlobTid, NULL ) ;
      UINT32 j ;
      for( j = 0 ; j < putLob ; j++ )
      {
         memset( lobReadBuf, 0, lobSize ) ;
         rlobarg.Buffer = lobReadBuf ;
         rc = pthread_create( &rlobTid, NULL, tReadLob, (void *)&rlobarg ) ;
         ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                                     << " rc =" << rc ;
         pthread_join( rlobTid, NULL ) ;
      }
   }
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   printf( "*=<Success to Concurrent Read different OID LOB and same OID LOB>=*\n" ) ;
   for( i = 0 ; i < putLob ; ++i )
   {
      rc = sdbRemoveLob( cl, &(OIDS[i]) ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Remove Lob $Oid = "
                                  << ", rc = " << rc ;
   }
   printf( "*=<Success to Remove %d LOB>=*\n", putLob ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object concurrent Seek Read LOB.
*               Test large object concurrent Get size of LOB.
*               { TestCase Number : LOB.CONCURRENT_TEST.006 }
*               { TestCase Number : LOB.CONCURRENT_TEST.007 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( ConcurrentTest, lobConcurrentSeekRead )
{
   const UINT64 lobSize = 1024*1024*16 ;
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR *readBuf = NULL ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 putLob = 10 ;
   bson_oid_t oid, OIDS[putLob] ;
   bson obj ;
   pthread_t wlobTid, rlobTid;
   struct Lob wlobarg ;
   struct Lob rlobarg ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to run create CL, rc = " << rc  ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Write different LOB by using diffrent data
   UINT32 i, size = 0 ;
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      // Write Large Object
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || (putLob-1) == i )
         bson_print( &obj ) ;  // Print the oid
      OIDS[i] = oid ;
      wlobarg.CL = cl ;
      wlobarg.OID = oid ;
      wlobarg.Buffer = lobWriteBuf ;
      wlobarg.size = 0 ;
      rc = pthread_create( &wlobTid, NULL, tWriteLob, (void *)&wlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                               << " rc =" << rc ;
      pthread_join( wlobTid, NULL ) ;
      if( 0 == i )
      {
         size = strlen( lobWriteBuf ) ;
         printf( "write Size = %ld\n SIZE = %ld\n", strlen( lobWriteBuf ),
                                                  strlen( wlobarg.Buffer ) ) ;
      }
   }
   printf( "*=<Success to Put %d LOB>=*\n", putLob ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   if( NULL == ( lobReadBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Concurrent Read Lob different OID and same OID
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobReadBuf, 0, lobSize ) ;
      rlobarg.CL = cl ;
      rlobarg.OID = OIDS[i] ;
      rlobarg.Buffer = lobReadBuf ;
      rlobarg.size = size ;
      printf( "Error will ocure here\n" ) ;
      rc = pthread_create( &rlobTid, NULL, tSeekReadLob, (void *)&rlobarg ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                                  << " rc =" << rc ;
      tGetLobSize( (void *)&rlobarg ) ;
      printf( "Size of lobReadBuf = %ld\n", strlen(rlobarg.Buffer) ) ;
      if( rlobarg.size != strlen(lobReadBuf) )
      {
         FILE *wfile ;
         if( NULL == ( wfile = fopen( "tmpLob.txt", "w+" ) ) )
         {
            perror( "tmpLob.txt" ) ;
            ASSERT_TRUE( false ) ;
         }
         fwrite( lobReadBuf, strlen(lobReadBuf), 1, wfile ) ;
         fclose( wfile ) ;
         ASSERT_TRUE( rlobarg.size == lobSize ) << "Wrong Write Lob, " << lobSize
                                                << " Size = " << rlobarg.size ;
      }
      pthread_join( rlobTid, NULL ) ;
      UINT32 j ;
      for( j = 0 ; j < putLob ; j++ )
      {
         memset( lobReadBuf, 0, lobSize ) ;
         rlobarg.Buffer = lobReadBuf ;
         rc = pthread_create( &rlobTid, NULL, tReadLob, (void *)&rlobarg ) ;
         ASSERT_TRUE( SDB_OK == rc ) << "Failed create thread for write LOB,"
                                     << " rc =" << rc ;
         pthread_join( rlobTid, NULL ) ;
      }
   }
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   printf( "*=<Success to Concurrent Read different OID LOB and same OID LOB>=*\n" ) ;
   for( i = 0 ; i < putLob ; ++i )
   {
      rc = sdbRemoveLob( cl, &(OIDS[i]) ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Remove Lob $Oid = "
                                  << ", rc = " << rc ;
   }
   printf( "*=<Success to Remove %d LOB>=*\n", putLob ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : 1.Test large object concurrent Read different LOB.
*               { TestCase Number : LOB.CONCURRENT_TEST.009 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( ConcurrentTest, lobConcurrentWriteAndRead )
{
   const UINT64 lobSize = 1024*1024*16 ;
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR *readBuf = NULL ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 putLob = 10 ;
   bson_oid_t oid, OIDS[putLob] ;
   bson obj ;
   pthread_t wlobTid, rlobTid;
   struct Lob wlob ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to run create CL, rc = " << rc  ;
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   if( NULL == ( lobReadBuf = (CHAR*)malloc( lobSize) ))
   {
      ASSERT_TRUE( false )<< "Failed to allocate memory for lob Read buffer" ;
   }
   UINT32 i ;
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || (putLob-1) == i )
         bson_print( &obj ) ;  // Print the oid
      wlob.CL = cl ;
      wlob.OID = oid ;
      wlob.Buffer = lobWriteBuf ;
      wlob.size = 0 ;
      // Concurrent Write LOB
      printf( "the %d count writeLob\n", i ) ;
      rc = pthread_create( &wlobTid, NULL, tWriteLob, ( void * )&wlob ) ;
      ASSERT_TRUE( SDB_OK == rc )<< "Failed to create thread for Write LOB, "
                                    "rc = " << rc ;
      usleep( 10000 ) ;
      // Concurrent Read LOB
      memset( lobReadBuf, 0, lobSize ) ;
      wlob.Buffer = lobReadBuf ;
      printf( "the %d count readLob\n", i ) ;
      rc = pthread_create( &rlobTid, NULL, tReadLob, ( void * )&wlob ) ;

      pthread_join( wlobTid, NULL ) ;
      pthread_join( rlobTid, NULL ) ;

   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   for( i = 0 ; i < putLob ; ++i )
   {
      rc = sdbRemoveLob( cl, &(OIDS[i]) ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Remove Lob $Oid = "
                                  << ", rc = " << rc ;
   }
   printf( "*=<Success to Remove %d LOB>=*\n", putLob ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

