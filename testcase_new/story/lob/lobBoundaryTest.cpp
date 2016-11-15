/*******************************************************************************
*@Description : All LOB Boundary Function Testcase.Description of testcase
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
*               write "" lob success and read "" lob failed
*               { TestCase Number : LOB.BOUNDARY_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( BoundaryTest, lobWriteZeroSizeAndRead )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *preName = "LOB_Write_Zero_Size_And_Read" ;
   CHAR lobReadBuf[1] = { 0 } ;
   CHAR clName[50] ;
   bson_oid_t oid, oid1;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME ) ;
   ASSERT_TRUE( SDB_OK == rc ) ;
   bson_oid_gen( &oid ) ;
   // Write LOB while "lobWriteBuf=NULL"
   CHAR *lobWriteBuf = NULL ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobWriteBuf, 0 ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Success to inspect write lob buffer equal null>=*\n" ) ;
   // Write LOB while 'lobWriteBuf=""'
   lobWriteBuf = "" ;
   lob = SDB_INVALID_HANDLE ;
   bson_oid_gen( &oid1 ) ;
   rc = sdbOpenLob( cl, &oid1, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobWriteBuf, 0 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read lob lobWriteBuf = ""
   rc = sdbOpenLob( cl, &oid1, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   UINT32 readLen = 20 ;
   rc = sdbReadLob( lob, 0, lobReadBuf, &readLen ) ;  // throw SDB_EOF ????
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( readLen, 0 ) ;  // read size equal 0

   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
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
   const CHAR *preName = "L0B_Size_About__256K" ;
   CHAR *lobBuffer = NULL ;
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
   CHAR *seekBuf = NULL ;
   CHAR fname1[15], fname2[15] ;
   bson_oid_t oid ;
   bson obj ;
   FILE *pFile ;

   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR *)calloc( bufLen[6], sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   if( NULL == ( seekBuf = (CHAR *)calloc( bufLen[6], sizeof(char) ) ) )
   {
      perror( "seekBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   UINT32 i ;
   for( i = 0 ; i < sizeof(bufLen)/sizeof(bufLen[0]) ; ++i )
   {
      printf( "*=<Begin to Write Lob, Size = %llu\n>=*", bufLen[i] ) ;
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      bson_destroy( &obj ) ;
      genLobData( lobBuffer, bufLen[i] ) ;

      memcpy( seekBuf, lobBuffer+seekSize[i], bufLen[i]-seekSize[i] ) ;
      rc = md5Code( seekBuf, wMd5, bufLen[i]-seekSize[i] ) ;
      printf( "===>write MD5 : %s, size : %d\n", wMd5, bufLen[i]-seekSize[i] ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Write LOB
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, bufLen[i] ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Read LOB
      memset( lobBuffer, 0, bufLen[6] ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize[i], SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, bufLen[i], lobBuffer, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, rMd5, readLen ) ;
      printf( "===>read MD5 : %s, size : %d\n", rMd5, readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Check
      sprintf( fname1, "%s%d", "writeLob.buf", i ) ;
      sprintf( fname2, "%s%d", "readLob.buf", i ) ;
      if( strcmp( wMd5, rMd5 ) )  // get error md5???
      {
         pFile = fopen( fname1, "w+" ) ;
         fwrite( seekBuf, sizeof(char), sizeof(seekBuf), pFile ) ;
         fclose( pFile ) ;
         pFile = fopen( fname2, "w+" ) ;
         fwrite( lobBuffer, sizeof(char), sizeof(lobBuffer), pFile ) ;
         fclose( pFile ) ;
      }
      ASSERT_STREQ( wMd5, rMd5 ) << "Wrong Write Lob and Read Lob"
                                  << "\nWrite Md5 = " << wMd5
                                  << "\nRead Md5 = " << rMd5 ;
      printf( "*=<Check MD5 SUCCESS>=*\n" ) ;
      diffSize = bufLen[i] - readLen ;
      ASSERT_EQ( seekSize[i], diffSize ) << "Wrong Write Lob and Read Lob"
                                             << "\nWrite Size = " << bufLen[i]
                                             << "\nRead Size = " << readLen ;
      printf( "*=<Check Size SUCCESS>=*\n" ) ;
      // memset the memory
      memset( seekBuf, 0, bufLen[6] ) ;
      memset( lobBuffer, 0, bufLen[6] ) ;

      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   free( seekBuf ) ;
   seekBuf = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
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
   CHAR *lobBuffer = NULL ;
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
   CHAR *seekBuf = NULL ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR *)calloc( bufLen[8], sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   if( NULL == ( seekBuf = (CHAR *)calloc( bufLen[8], sizeof(char) ) ) )
   {
      perror( "seekBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   UINT32 i ;
   for( i = 0 ; i < sizeof(bufLen)/sizeof(bufLen[0]) ; ++i )
   {
      printf( "*=<Begin to Write Lob, Size = %llu\n>=*", bufLen[i] ) ;
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      genLobData( lobBuffer, bufLen[i] ) ;

      memcpy( seekBuf, lobBuffer+seekSize[i], bufLen[i]-seekSize[i] ) ;
      rc = md5Code( seekBuf, wMd5, bufLen[i]-seekSize[i] ) ;
      printf( "===>write MD5 : %s, size : %d\n", wMd5, bufLen[i]-seekSize[i] ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Write LOB
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, bufLen[i] ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Read LOB
      memset( lobBuffer, 0, bufLen[8] ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize[i], SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, bufLen[i], lobBuffer, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, rMd5, readLen ) ;
      printf( "===>write MD5 : %s, size : %d\n", rMd5, readLen ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      // if md5 no equal, then printf file
      FILE *pFile ;
      CHAR fname1[15], fname2[15] ;
      sprintf( fname1, "%s%d", "writeLob.buf", i ) ;
      sprintf( fname2, "%s%d", "readLob.buf", i ) ;
      if( strcmp( wMd5, rMd5 ) )  // get error md5???
      {
         pFile = fopen( fname1, "w+" ) ;
         fwrite( seekBuf, sizeof(char), sizeof(seekBuf), pFile ) ;
         fclose( pFile ) ;
         pFile = fopen( fname2, "w+" ) ;
         fwrite( lobBuffer, sizeof(char), sizeof(lobBuffer), pFile ) ;
         fclose( pFile ) ;
      }
      // Check
      ASSERT_STREQ( wMd5, rMd5 ) << "Wrong Write Lob and Read Lob"
                                  << "\nWrite Md5 = " << wMd5
                                  << "\nRead Md5 = " << rMd5 ;
      printf( "*=<Check MD5 SUCCESS>=*\n" ) ;
      printf( "seekSize[i] = %d\n", seekSize[i] ) ;
      diffSize = bufLen[i] - readLen ;
      ASSERT_EQ( seekSize[i], diffSize ) << "Wrong Write Lob and Read Lob"
                                          << "\nWrite Size = " << bufLen[i]
                                          << "\nRead Size = " << readLen ;
      printf( "*=<Check Size SUCCESS>=*\n" ) ;
      // memset the memory
      memset( seekBuf, 0, bufLen[6] ) ;
      memset( lobBuffer, 0, bufLen[6] ) ;

      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   free( seekBuf ) ;
   seekBuf = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : create collection space specify lob page size, which located
*               in { 0, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288}
*               abnormal testcase in lobAbnormalTestcase.cpp
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( BoundaryTest, lobSpecifyLobPageSize )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbConnectionHandle db1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cs1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cl1 = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*513 ;
   const CHAR *prefName = "Specify_Lob_Page_Size" ;
   CHAR pref[30] = { 0 } ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   UINT64 putNum = 20 ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   INT32 pageSz[9] = { 0, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288} ;
   bson_oid_t oids[putNum] ;
   bson startObj, endObj ;
   bson lobPzObj ;
   INT32 i ;

   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   for( i = 0 ; i < 9 ; ++i )
   {
      bson_init( &lobPzObj ) ;
      bson_append_int( &lobPzObj, "LobPageSize", pageSz[i] ) ;
      bson_append_finish_object( &lobPzObj ) ;
      sprintf( pref, "%s%d", prefName, i ) ;
      rc = lobCreateCollectionPz( &db, &cs, pref, &cl, clName, &lobPzObj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      bson_print( &lobPzObj ) ;
      bson_destroy( &lobPzObj ) ;
      genLobData( lobBuffer, lobSize ) ;
      rc = md5Code( lobBuffer, md5, ENCRYTED_STR_LEN ) ;   // get md5 code
      ASSERT_EQ( SDB_OK, rc ) ;
      // write lob
      splitLobWrite( cl, lobBuffer, lobSize, putNum, oids ) ;
      printf( "*=<write lob successful>=*\n" ) ;
      // read lob and inspect md5
      splitLobRead( cl, lobBuffer, lobSize, putNum, oids, md5 ) ;
      printf( "*=<read lob successful>=*\n" ) ;
      // remove lob
      splitLobRemove( cl, lobBuffer, lobSize, putNum, oids ) ;
      printf( "*=<remove lob successful>=*\n" ) ;

      // clean in the end
      //rc = sdbDropCollection( cs, clName ) ;
      //ASSERT_EQ( SDB_OK, rc ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
