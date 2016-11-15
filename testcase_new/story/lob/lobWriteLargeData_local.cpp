/*******************************************************************************
*@Description : All Write much big data LOB Testcase.Description of testcase in
*               directory
*               [ internal_doc/03.开发设计/Story_lob.docx -> 七、验收测试用例 ]
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gtest/gtest.h"
#include "client.h"
#include "testcommon.h"
#include <limits.h>
#include <unistd.h>
#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )

/*******************************************************************************
*@Description : Write one very large object data.
*               { TestCase Number : LOB.MUCHBIGER_TEST. }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( LargeDataTest, writeOneVeryLargeData )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   const UINT32 putNum = 10 ;
   UINT64 sumSize = lobSize*putNum ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write One Large Data
   bson_oid_gen( &oid ) ;
   rc =sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR*)malloc( lobSize ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   UINT32 i, size = 0 ;
   UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
   md5_state_t mst ;
   md5_init( &mst ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      genLobData( lobBuffer, lobSize ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      md5_append( &mst, (const md5_byte_t *)lobBuffer, lobSize ) ;
   }
   md5_finish( &mst, md5digest ) ;
   CHAR *wmd5 = wMd5 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
   {
      snprintf( wmd5, 3, "%02x", md5digest[i] ) ;
      wmd5 += 2 ;
   }
   printf( "*=<Write MD5 = %s>=*\n", wMd5 ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Get Size and Create Time
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ; // Size
   ASSERT_EQ( SDB_OK, rc ) ;
   UINT64 millis = 0 ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Put reletive information in nomal record
   bson_init( &obj ) ;
   bson_append_oid( &obj, "LobOid", &oid ) ;
   bson_append_long( &obj, "LobSize", getSize ) ;
   bson_append_timestamp2( &obj, "LobTimestamp", millis/1000, millis%1000 ) ;
   bson_append_string( &obj, "LobMd5", wMd5 ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;
   rc = sdbInsert( cl, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   //printf( "Get size = %d\n", getSize ) ;
   // compare get size and real sum size
   ASSERT_EQ( getSize, sumSize ) << "Write ERROR Data"
                                 << "\nGet Lob Size = " << getSize
                                 << "\nPut Lob Size = " << sumSize ;
   // Read Lob and Get MD5
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   UINT64 seekSize = 0 ;
   md5_init( &mst ) ;
   md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      seekSize += lobRead ;
      md5_append( &mst, (const md5_byte_t *)lobBuffer, lobRead ) ;
   }
   md5_finish( &mst, md5digest ) ;
   CHAR *rmd5 = rMd5 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
   {
      snprintf( rmd5, 3, "%02x", md5digest[i] ) ;
      rmd5 += 2 ;
   }
   // compare write md5 and read md5
   ASSERT_STREQ( wmd5, rmd5 ) << "Write Lob Data is not equal Read Lob Data"
                              << "Write MD5 = " << wmd5
                              << "Read MD5 = " << rmd5 ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;

   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : write multi large data to sdb.
*               { TestCase Number : LOB.ACCEPTANCE_TEST. }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( LargeDataTest, writeMultiLargeData )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*20 ;   // one lob size
   const CHAR *prefixClName = "Lob_Write_Multi_Large_Data" ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   const UINT32 putNum = 1 ;
   UINT64 sumSize = lobSize*putNum ;
   CHAR *wMd5s[putNum] = { 0 } ;
   bson_oid_t oid, oids[putNum] ;
   bson obj ;
   UINT32 i = 0 ;
   UINT32 size = 0 ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR*)malloc( lobSize ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // Write One Large Data
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;  // init memery
      genLobData( lobBuffer, lobSize ) ;
      bson_oid_gen( &oid ) ;
      oids[i] = oid ;
      rc =sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
      md5_state_t mst ;
      md5_init( &mst ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      EXPECT_EQ( SDB_OK, rc ) ;
      if( SDB_NOSPC == rc )
      {
         rc = sdbDropCollectionSpace( db, COMMCSNAME ) ;
         ASSERT_EQ( SDB_OK, rc ) ;
         goto done ;
      }
      md5_append( &mst, (const md5_byte_t *)lobBuffer, lobSize ) ;
      md5_finish( &mst, md5digest ) ;
      CHAR *wmd5 = wMd5 ;
      for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
      {
         snprintf( wmd5, 3, "%02x", md5digest[i] ) ;
         wmd5 += 2 ;
      }
      printf( "*=<Write MD5 = %s>=*\n", wMd5 ) ;
      wMd5s[i] = wMd5 ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Get Size and Create Time
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      SINT64 getSize = 0 ;
      rc = sdbGetLobSize( lob, &getSize ) ; // Size
      ASSERT_EQ( SDB_OK, rc ) ;
      UINT64 millis = 0 ;
      rc = sdbGetLobCreateTime( lob, &millis ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Put reletive information in nomal record
      bson_init( &obj ) ;
      bson_append_oid( &obj, "LobOid", &oid ) ;
      bson_append_long( &obj, "LobSize", getSize ) ;
      bson_append_timestamp2( &obj, "LobTimestamp", millis/1000, millis%1000 ) ;
      bson_append_string( &obj, "LobMd5", wMd5 ) ;
      bson_append_finish_object( &obj ) ;
      rc = sdbInsert( cl, &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      //printf( "Get size = %d\n", getSize ) ;
      // compare get size and real sum size
      ASSERT_EQ( getSize, lobSize ) << "Write ERROR Data"
                                    << "\nGet Lob Size = " << getSize
                                    << "\nPut Lob Size = " << sumSize ;
   }
   printf( "*=<write lob over>=*\n" ) ;
   // Read Lob and Get MD5
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &(oids[i]), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      UINT64 seekSize = 0 ;
      md5_state_t mst1 ;
      md5_init( &mst1 ) ;
      UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
      md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      //seekSize += lobRead ;
      md5_append( &mst1, (const md5_byte_t *)lobBuffer, lobRead ) ;
      md5_finish( &mst1, md5digest ) ;
      CHAR *rmd5 = rMd5 ;
      for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
      {
         snprintf( rmd5, 3, "%02x", md5digest[i] ) ;
         rmd5 += 2 ;
      }
      // compare write md5 and read md5
      ASSERT_STREQ( wMd5s[i], rmd5 ) << "Write Lob Data is not equal Read Lob Data"
                                     << "Write MD5 = " << wMd5s[i]
                                     << "Read MD5 = " << rmd5 ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;

done:
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object write and then read.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( LargeDataTest, lobWriteRead )
{
/*
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*20 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR cwMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR crMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *readBuf = NULL ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   UINT8 md5digest [ SDB_MD5_DIGEST_LENGTH ] = {0} ;
   bson_oid_t oid ;
   bson obj ;

   if( NULL == ( lobBuffer = (CHAR*)malloc( lobSize ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Seek Read File
   md5_state_t pms ;
   UINT64 seekSize = 0 ;
   md5_init( &pms ) ;
   size_t rclen = 0 ;
   if( NULL == (readBuf = (CHAR*)malloc( lobSize )) )
   {
      perror( "readBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   do
   {
      //fseek( sFile, seekSize, SEEK_SET ) ;
      rclen = fread( readBuf, 1, strlen(lobBuffer)/4, sFile ) ;
      md5_append( &pms, ( const md5_byte_t *)readBuf, rclen ) ;
      //seekSize += 50000 ;
   }while( rclen > 0 ) ;
   free( readBuf ) ;
   readBuf = NULL ;
   fclose( sFile ) ;
   CHAR *wmd5 = cwMd5 ;
   md5_finish( &pms, md5digest ) ;
   UINT32 i = 0 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH; i++ )
   {
      printf( "COUNT = %d\n", i ) ;
      snprintf( wmd5, 3, "%02x", md5digest[i] ) ;
      wmd5 += 2 ;
   }
   printf( "=================================================\n" ) ;
   printf( "Split MD5 = %s\n", cwMd5 ) ;
   printf( "=================================================\n" ) ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc )  ;
   printf( "========================================\n" ) ;
   printf( "Write MD5 = %s\n", wMd5 ) ;
   printf( "========================================\n" ) ;
   rc = sdbWriteLob( lob, lobBuffer, strlen(lobBuffer) ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   if( NULL == (lobBuffer = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // Read Large Object
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, rMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from read
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, strlen(lobBuffer), lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Read LOB OVER>=*\n" ) ;
   // Check the write and read is correct or not
   ASSERT_STREQ( wMd5, rMd5 ) << "*->ERROR, Read wrong data from LOB\n"
                              << "*->Write Lob Md5 = " << wMd5
                              << "\n*->Read  Lob Md5 =" << rMd5 ;
   printf( "*=<Correct LOB Write and LOB Read>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
*/
}

/*******************************************************************************
*@Description : Test multi lob and do hash split
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( LargeDataTest, writeOneVeryLargeDataThenSplit )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*20 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read_Then_Split" ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   const UINT32 putNum = 550 ;
   UINT64 sumSize = lobSize*putNum ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write One Large Data
   bson_oid_gen( &oid ) ;
   rc =sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR*)malloc( lobSize ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   UINT32 i, size = 0 ;
   UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
   md5_state_t mst ;
   md5_init( &mst ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      genLobData( lobBuffer, lobSize ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      md5_append( &mst, (const md5_byte_t *)lobBuffer, lobSize ) ;
   }
   md5_finish( &mst, md5digest ) ;
   CHAR *wmd5 = wMd5 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
   {
      snprintf( wmd5, 3, "%02x", md5digest[i] ) ;
      wmd5 += 2 ;
   }
   printf( "*=<Write MD5 = %s>=*\n", wMd5 ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Get Size and Create Time
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ; // Size
   ASSERT_EQ( SDB_OK, rc ) ;
   UINT64 millis = 0 ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Put reletive information in nomal record
   bson_init( &obj ) ;
   bson_append_oid( &obj, "LobOid", &oid ) ;
   bson_append_long( &obj, "LobSize", getSize ) ;
   bson_append_timestamp2( &obj, "LobTimestamp", millis/1000, millis%1000 ) ;
   bson_append_string( &obj, "LobMd5", wMd5 ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;
   rc = sdbInsert( cl, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   //printf( "Get size = %d\n", getSize ) ;
   // compare get size and real sum size
   ASSERT_EQ( getSize, sumSize ) << "Write ERROR Data"
                                 << "\nGet Lob Size = " << getSize
                                 << "\nPut Lob Size = " << sumSize ;
   // Read Lob and Get MD5
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   UINT64 seekSize = 0 ;
   md5_init( &mst ) ;
   md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      seekSize += lobRead ;
      md5_append( &mst, (const md5_byte_t *)lobBuffer, lobRead ) ;
   }
   md5_finish( &mst, md5digest ) ;
   CHAR *rmd5 = rMd5 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
   {
      snprintf( rmd5, 3, "%02x", md5digest[i] ) ;
      rmd5 += 2 ;
   }
   // compare write md5 and read md5
   ASSERT_STREQ( wmd5, rmd5 ) << "Write Lob Data is not equal Read Lob Data"
                              << "Write MD5 = " << wmd5
                              << "Read MD5 = " << rmd5 ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   /**********************************
   * hash split
   **********************************/
   bson startObj, endObj ;
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 300 ) ;
   bson_append_int( &endObj, "Partition", 700 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;
   // must be CL name like : foo.bar
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "=>get destGroup = %s, =>get srcGroup = %s\n", srcGroup, dstGroup ) ;
      rc = sdbSplitCollection( cl, srcGroup, dstGroup, &startObj, &endObj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "success to split large data\n" ) ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;

   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
