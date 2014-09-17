/*******************************************************************************
*@Description : All LOB Acceptance Testcase.Description of testcase in directory
*               [ trunk/testprocess/1-process/大对象测试/Testing_LOB.xlsx ]
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
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
*@Description : Test Write various Types Data, such as zip/jpj/mp4/ape/pdf.
*               { TestCase Number : LOB.VARIOUS.TYPES_DATA_TEST.001 }
*               { TestCase Number : LOB.VARIOUS.TYPES_DATA_TEST.002 }
*               { TestCase Number : LOB.VARIOUS.TYPES_DATA_TEST.003 }
*               { TestCase Number : LOB.VARIOUS.TYPES_DATA_TEST.004 }
*               { TestCase Number : LOB.VARIOUS.TYPES_DATA_TEST.005 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( variousTypesData, lobWriteRead )
{
/*
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1]   = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1]   = { 0 } ;
   const CHAR *prefName = "LOB_Various_Types_Data" ;
   const UINT64 allocSize = 100*1024*1024 ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[64] = { 0 } ;
   CHAR *fileName[6] = { "./LOB_TEST_FILES/testLobCompress.zip",
                         "./LOB_TEST_FILES/testLobJavaBook.pdf",
                         "./LOB_TEST_FILES/testLobPicture.jpg",
                         "./LOB_TEST_FILES/testLobVideo.mp4",
                         "./LOB_TEST_FILES/testLobVoice.ape" } ;
   const CHAR *wfileName[6] = { "./LOB_TEST_FILES/1TestLobCompress.zip",
                                "./LOB_TEST_FILES/1TestLobJavaBook.pdf",
                                "./LOB_TEST_FILES/1TestLobPicture.jpg",
                                "./LOB_TEST_FILES/1TestLobVideo.mp4",
                                "./LOB_TEST_FILES/1TestLobVoice.ape" } ;
   UINT32 readLen = 0 ;
   SINT64 getSize = 0 ;
   size_t frSize = 0 ;
   size_t fwSize = 0 ;
   size_t fsize = 0 ;
   UINT32 i, j ;
   FILE *file, *wfile ;
   bson_oid_t oid ;
   bson obj ;

   rc = tCreateCollection( &db, &cs, prefName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to create test CL, rc = " << rc ;
   if( NULL == ( lobBuffer = (CHAR *)malloc( allocSize ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   for( i = 0 ; i < 5 ; ++i )
   {
      printf( "*=====< Read File Name = %s >=====*\n", fileName[i] ) ;
      printf( "*=====< Write File Name = %s >=====*\n", wfileName[i] ) ;
      if( NULL == ( file = fopen( fileName[i], "r+" ) ) )
      {
         perror( "Open fileName" ) ;
         ASSERT_TRUE( false) << "file Name = " << fileName[i];
      }
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$OID", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Open LOB in Write, rc = " << rc ;
      UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
      md5_state_t mst ;
      md5_init( &mst ) ;
      while( !feof(file) )
      {
         fwSize = fread( lobBuffer, 1, allocSize, file ) ;
         //rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;
         md5_append( &mst, (const md5_byte_t *)lobBuffer, fwSize ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to get Md5 code, rc = " << rc ;
         rc = sdbWriteLob( lob, lobBuffer, fwSize ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to Write LOB, rc = " << rc ;
         memset( lobBuffer, 0, fwSize ) ;
         fsize += fwSize ;
      }
      md5_finish( &mst, md5digest ) ;
      CHAR *md5 = wMd5 ;
      for( j = 0 ; j < SDB_MD5_DIGEST_LENGTH ; ++j )
      {
         snprintf( md5, 3, "%02x", md5digest[j] ) ;
         md5 += 2 ;
      }
      printf( "Read File Size = %d\n", fsize ) ;
      //UINT32 lenRead = strlen(lobBuffer)  ;
      fclose( file ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in Write, rc = " << rc ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Open LOB in Read, rc = " << rc ;
      rc = sdbGetLobSize( lob, &getSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Get LOB Size, rc = " << rc ;
      printf( "Get LOB Size = %d\n", getSize ) ;
      printf( "*=<Write LOB OVER>=*\n" ) ;
      //memset( lobBuffer, 0, allocSize ) ;
      printf( "=================wfileName = %s=============\n", wfileName[i] ) ;
      if( NULL == ( wfile = fopen( wfileName[i], "w+b") ) )
      {
         perror( "wfileName" ) ;
         ASSERT_TRUE( false ) << "Write fileName = " << wfileName[j] ;
      }
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Open LOB in Read, rc = " << rc ;
      fsize = 0 ;
      md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
      md5_state_t wmst ;
      md5_init( &wmst ) ;
      for( j = 0 ; j <= getSize/allocSize ; ++j )
      {
         //seekSize += allocSize ;
         rc = sdbReadLob( lob, allocSize, lobBuffer, &readLen ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to Read LOB, rc = " << rc ;
         fwSize = fwrite( lobBuffer, readLen, 1, wfile ) ;
         fsize += fwSize ;
         md5_append( &wmst, (const md5_byte_t *)lobBuffer, readLen ) ;
      }
      md5_finish( &wmst, md5digest ) ;
      md5 = NULL ;
      md5 = rMd5 ;
      for( j = 0 ; j < SDB_MD5_DIGEST_LENGTH ; ++j )
      {
         snprintf( md5, 3, "%02x", md5digest[j] ) ;
         md5 += 2 ;
      }
      fclose( wfile ) ;
      printf( "Read LOB File Size = %d\n", readLen ) ;
      printf( "Write File Size = %d\n", fsize ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in Read, rc = " << rc ;
      //rc = md5Code( lobBuffer, rMd5, ENCRYTED_STR_LEN ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get Md5 code, rc = " << rc ;
      printf( "*=<Read LOB OVER>=*\n" ) ;
      // Check Value
      ASSERT_STREQ( wMd5, rMd5 ) << "Write Md5 not equal Read Md5"
                                 << "\nWrite LOB MD5 Code = " << wMd5
                                 << "\nRead LOB MD5 Code = " << rMd5 ;
      printf( "Write LOB MD5 Code = %s\nRead LOB MD5 Code = %s\n", wMd5, rMd5 ) ;
      printf( "*=<Check LOB MD5 OVER>=*\n" ) ;
   }

   free( lobBuffer ) ;
   lobBuffer = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Drop Collection, rc = " << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
*/
}

/*******************************************************************************
*@Description : Write variouty LOB in SDB.
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( userScenarioTest, writeLobAndPutInOneRecord )
{
/*
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR* preName = "Write_Lob_And_Then_Create_One_Record" ;
   CHAR clName[70] = { 0 } ;
   const UINT64 lobSize = 16*1024*1024 ;
   CHAR* buffer = NULL ;
   CHAR* lobWriteBuf = NULL ;
   const UINT32 putNum = 10000000 ;
   bson_oid_t oid ;
   bson obj ;
   bson_timestamp_t tm ;

   rc = tCreateCollection( &db, &cs, preName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to create collection, rc = " << rc ;
   if( NULL == ( buffer = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "malloc_buffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   if( NULL == ( lobWriteBuf = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "malloc_lobWriteBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   memset( buffer, 0, lobSize ) ;
   genLobData( buffer, lobSize ) ;
   UINT32 i ;
   for( i = 0 ; i < putNum ; ++i )
   {
      bson_oid_gen( &oid ) ;
      memset( lobWriteBuf, 0, lobSize ) ;
      strncpy( lobWriteBuf, buffer, lobSize ) ;
      // Write Lob
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Create Lob, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Write Lob, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Close Lob in Write, rc = " << rc ;
      // Get Lob Size and Get Lob Create Time
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Open Lob Read, rc = " << rc ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Get Lob Size, rc = " << rc ;
      UINT64 millis = 0 ;
      rc = sdbGetLobCreateTime( lob, &millis ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Get Lob Create Time, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Close Lob in Get Size, rc = " << rc ;
      printf( "milli seconds = %ld\n", millis ) ;
      tm.t = millis/1000 ;
      tm.i = ( millis%1000 ) * 1000 ;
      printf( "time = %d\n", tm.i ) ;
      // Put Size and Time in Sdb
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_int( &obj, "LobSize", sizeWrite ) ;
      bson_append_timestamp( &obj, "LobCreateTime", &tm ) ;
      bson_append_finish_object( &obj ) ;
      rc = sdbInsert( cl, &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Insert Data to Sdb, rc = " << rc ;

   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( buffer ) ;
   buffer = NULL ;
*/
}

/*******************************************************************************
*@Description : Put BIG file in Sdb.
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( variousTypesData, lobWriteMuch )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1]   = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1]   = { 0 } ;
   const CHAR *prefName = "LOB_Various_Types_Data" ;
   const UINT64 allocSize = 100*1024*1024 ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[64] = { 0 } ;
   CHAR *fileName[1] = { "./LOB_TEST_FILES/testLobCompress.zip" } ;
/*
   CHAR *fileName[6] = { "./LOB_TEST_FILES/testLobCompress.zip",
                         "./LOB_TEST_FILES/testLobJavaBook.pdf",
                         "./LOB_TEST_FILES/testLobPicture.jpg",
                         "./LOB_TEST_FILES/testLobVideo.mp4",
                         "./LOB_TEST_FILES/testLobVoice.ape" } ;
*/
   const CHAR *wfileName[6] = { "./LOB_TEST_FILES/1TestLobCompress.zip",
                                "./LOB_TEST_FILES/1TestLobJavaBook.pdf",
                                "./LOB_TEST_FILES/1TestLobPicture.jpg",
                                "./LOB_TEST_FILES/1TestLobVideo.mp4",
                                "./LOB_TEST_FILES/1TestLobVoice.ape" } ;
   UINT32 readLen = 0 ;
   SINT64 getSize = 0 ;
   size_t frSize = 0 ;
   size_t fwSize = 0 ;
   size_t fsize = 0 ;
   UINT32 i, j ;
   FILE *file, *wfile ;
   bson_oid_t oid ;
   bson obj ;
   bson_timestamp_t tm ;

   rc = tCreateCollection( &db, &cs, prefName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to create test CL, rc = " << rc ;
   if( NULL == ( lobBuffer = (CHAR *)malloc( allocSize ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }

   for( i = 0 ; i < 20 ; ++i )
   {
      printf( "*=====< Read File Name = %s >=====*\n", fileName[0] ) ;
      if( NULL == ( file = fopen( fileName[0], "r+" ) ) )
      {
         perror( "Open fileName" ) ;
         ASSERT_TRUE( false) << "file Name = " << fileName[0];
      }
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$OID", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Open LOB in Write, rc = " << rc ;
      UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
      md5_state_t mst ;
      md5_init( &mst ) ;
      while( !feof(file) )
      {
         fwSize = fread( lobBuffer, 1, allocSize, file ) ;
         //rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;
         md5_append( &mst, (const md5_byte_t *)lobBuffer, fwSize ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to get Md5 code, rc = " << rc ;
         rc = sdbWriteLob( lob, lobBuffer, fwSize ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to Write LOB, rc = " << rc ;
         memset( lobBuffer, 0, fwSize ) ;
         fsize += fwSize ;
      }
      md5_finish( &mst, md5digest ) ;
      CHAR *md5 = wMd5 ;
      for( j = 0 ; j < SDB_MD5_DIGEST_LENGTH ; ++j )
      {
         snprintf( md5, 3, "%02x", md5digest[j] ) ;
         md5 += 2 ;
      }
      //printf( "Read File Size = %d\n", fsize ) ;
      //UINT32 lenRead = strlen(lobBuffer)  ;
      fclose( file ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in Write, rc = " << rc ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Open LOB in Read, rc = " << rc ;
      rc = sdbGetLobSize( lob, &getSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Get LOB Size, rc = " << rc ;
      UINT64 millis = 0 ;
      rc = sdbGetLobCreateTime( lob, &millis ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Get Lob Create Time, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB, rc = " << rc ;
      tm.t = millis/1000 ;
      tm.i = ( millis%1000 ) * 1000 ;
      //printf( "time = %d\n", tm.i ) ;
      // Put Size and Time in Sdb
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_int( &obj, "LobSize", getSize ) ;
      bson_append_timestamp( &obj, "LobCreateTime", &tm ) ;
      bson_append_finish_object( &obj ) ;
      rc = sdbInsert( cl, &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Insert Data to Sdb, rc = " << rc ;

   }

   free( lobBuffer ) ;
   lobBuffer = NULL ;
   //rc = sdbDropCollection( cs, clName ) ;
   //ASSERT_EQ( SDB_OK, rc ) << "Failed to Drop Collection, rc = " << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
