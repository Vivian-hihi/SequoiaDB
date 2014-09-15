/*******************************************************************************
*@Description : All LOB Foundamental Function Testcase.Description of testcase
*               in directory--->
*               [ trunk/testprocess/1-process/大对象测试/Testing_LOB.xlsx ]
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "testcommon.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
/*******************************************************************************
*@Description : Test Write Large Object NULL in Sdb can correct.
*               { TestCase Number : LOB.BOUNDARY_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( BoundaryTest, lobWriteZeroSize )
{
   sdbConnectionHandle db = 0 ;
   sdbCSHandle cs = 0 ;
   sdbCollectionHandle cl = 0 ;
   sdbLobHandle lob = 0 ;
   INT32 rc = SDB_OK ;
   const CHAR *preName = "LOB_Write_Zero_Size" ;
   CHAR *lobWriteBuf = NULL ;
   CHAR lobReadBuf[1] = { 0 } ;
   CHAR clName[50] ;
   bson_oid_t oid ;
   bson obj ;

   rc = tCreateCollection( &db, &cs, preName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to create collection, rc = " << rc  ;
   bson_oid_gen( &oid ) ;
   bson_init( &obj) ;
   bson_append_oid( &obj, "$Oid", &oid) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   // Write LOB while "lobWriteBuf=NULL"
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to Create Lob, rc = " << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, 0 ) ;
   ASSERT_TRUE( SDB_INVALIDARG == rc ) << "Failed to Write Lob, rc = " << rc ;
   printf( "*=<Success to inspect write lob buffer equal null>=*\n" ) ;
   // Write LOB while 'lobWriteBuf=""'
   lobWriteBuf = "" ;
   struct basicOp basic ;
   basic.Cl = cl ;
   basic.writeLobBuf = lobWriteBuf ;
   basic.readLobBuf = lobReadBuf ;
   basic.sekSize = 0 ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to Drop Collection, rc = " << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Write Large Object 256k, Test Seek Read LOB is correct or not.
*               { TestCase Number : LOB.BOUNDARY_TEST.002 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( BoundaryTest, lobBoundarySlice256KB )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *preName = "L0B_File_Less_Than_256K" ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR *lobBuf = NULL ;
   const UINT64 bufLen[7] = { 256*1024-1, 256*1024, 256*1024+1, 256*1024+1024,
                             2*256*1024+1024, 10*256*1024, 100*256*1024+4096 } ;
   UINT32 readLen = 0 ;
   UINT64 diffSize = 0 ;
   CHAR clName[60] = { 0 } ;
   UINT64 writeSize = 0 ;
   const SINT64 seekSize[7] = { 256*1024-911, 256*1024-5692, 256*1024-98210,
                                256*1024-78543, 2*256*1024+17, 9*256*1024,
                                100*256*1024+1237 } ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;

   rc = tCreateCollection( &db, &cs, preName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Faile to create collection, rc = " << rc ;
   if( NULL == ( lobWriteBuf = (CHAR *)malloc( bufLen[2]*110 ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for Write Lob buffer,"
                           << " rc = " << rc ;
   }
   if( NULL == ( lobReadBuf = (CHAR *)malloc( bufLen[2]*110 ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for Read Lob buffer,"
                           << " rc = " << rc ;
   }
   if( NULL == ( lobBuf = (CHAR *)malloc( bufLen[2]*110 ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for Lob temp buffer,"
                           << " rc = " << rc ;
   }
   UINT32 i ;
   for( i = 0 ; i < sizeof(bufLen)/sizeof(bufLen[0]) ; ++i )
   {
      printf( "<<<<<start from [%d]>>>>>\n", i ) ;
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      genLobData( lobWriteBuf, bufLen[i] ) ;

      printf( "====>LEN %d<====\n", strlen(lobWriteBuf) ) ;
      memcpy( lobBuf, lobWriteBuf+seekSize[i], bufLen[i]-seekSize[i] ) ;

      rc = md5Code( lobBuf, wMd5, bufLen[i]-seekSize[i] ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to get Md5 code, rc = " << rc ;
      // Write LOB
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Open LOB in Write Lob, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, strlen(lobWriteBuf) ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Write Lob, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Close LOB in Write Lob, rc = " << rc  ;
      writeSize = strlen( lobWriteBuf ) ;
      // Read LOB
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Open Lob in Read Lob, rc = " << rc ;
      rc = sdbSeekLob( lob, seekSize[i], SDB_LOB_SEEK_SET ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Seek Lob, rc = " << rc ;
      rc = sdbReadLob( lob, strlen(lobWriteBuf), lobReadBuf, &readLen ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Read Lob, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Close Lob in Read Lob, rc = " << rc ;
      printf( "Read Size = %d\n", strlen(lobReadBuf) ) ;
      rc = md5Code( lobReadBuf, rMd5, readLen ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to get Md5 code, rc = " << rc ;
      // Check
      printf( "WMD5 = %s\nRMD5 = %s\n", wMd5, rMd5 ) ;
      ASSERT_STREQ( wMd5, rMd5 ) << "Wrong Write Lob and Read Lob"
                                  << "\nWrite Md5 = " << wMd5
                                  << "\nRead Md5 = " << rMd5 ;
                                  //<< "\nSeek Buf = " << lobBuf
                                  //<< "\nRead Buf = " << lobReadBuf ;
      printf( "*=<Check MD5 SUCCESS>=*\n" ) ;
      printf( "seekSize = %d\n", seekSize[i] ) ;
      diffSize = writeSize - readLen ;
      ASSERT_TRUE( seekSize[i] == diffSize ) << "Wrong Write Lob and Read Lob"
                                          << "\nWrite Size = " << writeSize
                                          << "\nRead Size = " << readLen ;
      printf( "*=<Check Size SUCCESS>=*\n" ) ;

      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Remove LOB, rc = " << rc ;
   }
   free( lobBuf ) ;
   lobBuf = NULL ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection, rc = " << rc ;
}

/*******************************************************************************
*@Description : Write Large Object 128M, Test Seek Read LOB is correct or not.
*               { TestCase Number : LOB.BOUNDARY_TEST.003 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( BoundaryTest, lobBoundarySlice128MB )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *preName = "L0B_File_Boundary_About_128MB" ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR *lobBuf = NULL ;
   const UINT64 bufLen[9] = { 128*1000*1000-1, 128*1024*1024-1, 128*1024*1024,
                              128*1000*1000, 128*1000*1000+1, 128*1024*1024+1,
                              130*1000*1000, 135*1024*1024, 300*1024*1024 } ;
   UINT32 readLen = 0 ;
   UINT64 diffSize = 0 ;
   CHAR clName[50] = { 0 } ;
   UINT64 writeSize = 0 ;
   const SINT64 seekSize[9] = { 128*1000*1000-1257, 128*1024*1024-50000,
                                128*1024*1024-100000, 128*1000*1000-7891234,
                                128*1000*1000-41324123, 128*1024*1024-54452343,
                                130*1000*1000-84674636, 135*1024*1024-95243523,
                                300*1024*1024-98765432 } ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;

   rc = tCreateCollection( &db, &cs, preName, &cl, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Faile to create collection, rc = " << rc ;
   if( NULL == ( lobWriteBuf = (CHAR *)malloc( bufLen[8]+1024 ) ) )
   {
      perror("lobWriteBuf") ;
      ASSERT_TRUE( false ) << "Failed to allocate memory for Write Lob buffer" ;
   }
   if( NULL == ( lobReadBuf = (CHAR *)malloc( bufLen[8]+1024 ) ) )
   {
      perror("lobReadBuf") ;
      ASSERT_TRUE( false ) << "Failed to allocate memory for Read Lob buffer," ;
   }
   if( NULL == ( lobBuf = (CHAR *)malloc( bufLen[8]+1024 ) ) )
   {
      perror("lobBuf") ;
      ASSERT_TRUE( false ) << "Failed to allocate memory for Lob temp buffer," ;
   }
   UINT32 i ;
   for( i = 0 ; i < sizeof(bufLen)/sizeof(bufLen[0]) ; ++i )
   {
      printf( "<<<<<start from [%d]>>>>>\n", i ) ;
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      genLobData( lobWriteBuf, bufLen[i] ) ;
      printf( "====>LEN %d<====\n", strlen(lobWriteBuf)/1024 ) ;
      memcpy( lobBuf, lobWriteBuf+seekSize[i], bufLen[i]-seekSize[i] ) ;
      rc = md5Code( lobBuf, wMd5, bufLen[i]-seekSize[i] ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to get Md5 code, rc = " << rc ;
      // Write LOB
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Open LOB in Write Lob, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, strlen(lobWriteBuf) ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Write Lob, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Close LOB in Write Lob, rc = " << rc  ;
      writeSize = strlen( lobWriteBuf ) ;
      // Read LOB
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Open Lob in Read Lob, rc = " << rc ;
      rc = sdbSeekLob( lob, seekSize[i], SDB_LOB_SEEK_SET ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Seek Lob, rc = " << rc ;
      rc = sdbReadLob( lob, strlen(lobWriteBuf), lobReadBuf, &readLen ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Read Lob, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Close Lob in Read Lob, rc = " << rc ;
      printf( "Read Size = %d\n", strlen(lobReadBuf) ) ;
      rc = md5Code( lobReadBuf, rMd5, readLen ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to get Md5 code, rc = " << rc ;
      // Check
      printf( "WMD5 = %s\nRMD5 = %s\n", wMd5, rMd5 ) ;
      ASSERT_STREQ( wMd5, rMd5 ) << "Wrong Write Lob and Read Lob"
                                  << "\nWrite Md5 = " << wMd5
                                  << "\nRead Md5 = " << rMd5 ;
      printf( "*=<Check MD5 SUCCESS>=*\n" ) ;
      printf( "seekSize[i] = %d\n", seekSize[i] ) ;
      diffSize = writeSize - readLen ;
      ASSERT_TRUE( seekSize[i] == diffSize ) << "Wrong Write Lob and Read Lob"
                                          << "\nWrite Size = " << writeSize
                                          << "\nRead Size = " << readLen ;
      printf( "*=<Check Size SUCCESS>=*\n" ) ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_TRUE( SDB_OK == rc ) << "Failed to Remove LOB, rc = " << rc ;
   }
   free( lobBuf ) ;
   lobBuf = NULL ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_TRUE( SDB_OK == rc ) << "Failed to drop collection, rc = " << rc ;
}
