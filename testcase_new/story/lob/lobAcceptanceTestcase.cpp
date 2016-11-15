/*******************************************************************************
*@Description : All LOB Acceptance Testcase.Description of testcase in directory
*               [ internal_doc/03.开发设计/Story_lob.docx -> 七、验收测试用例 ]
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <gtest/gtest.h>
#include <malloc.h>
#include <boost/thread.hpp>
#include <unistd.h>
#include "testcommon.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
/*******************************************************************************
*@Description : Test large object write and then read.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobWriteRead )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[64] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;   // auto generate data
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Read Large Object
   memset( lobBuffer, 0, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, rMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from read
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
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test write plenty of large object to sdb that is correct or not.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.002 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobWriteLargeSame )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Write_Large_Same" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[64] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 writeLobNum = 20 ;
   bson_oid_t oid ;
   bson obj1 ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Allocate Memory
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;   // Auto generate data
   // Put 100 same Lob in SDB
   UINT32 i ;
   for( i = 0 ; i < writeLobNum ; ++i )
   {
      // Write large file to SDB
      bson_oid_gen( &oid ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Get the Size of Lob
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      ASSERT_EQ( sizeWrite, lobSize ) << "ERROR, Write wrong data to Sdb"
                                      << "Put Lob Size = " << lobSize
                                      << "Get Lob Size = " << sizeWrite ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Write plenty of large object to sdb and then read this
*               object,Testing Read operation is available or not.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.003 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobReadLargeSame )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Read_Large_Same_Data" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[64] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 writeLobNum = 20 ;
   CHAR *md5s[writeLobNum] = { 0 } ;
   bson_oid_t oid, oids[writeLobNum] ;
   bson obj1 ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Allocate Memory
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   // Put 100 same Lob in SDB
   UINT32 i ;
   for( i = 0 ; i < writeLobNum ; ++i )
   {
      // Write large file to SDB
      bson_oid_gen( &oid ) ;
      oids[i] = oid ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Get the Size of Lob
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      ASSERT_EQ( sizeWrite, lobSize ) << "ERROR, Write wrong data to Sdb"
                                      << "Put Lob Size = " << lobSize
                                      << "Get Lob Size = " << sizeWrite ;
      rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      // Get Size again and Create Time
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
      bson_init( &obj1 ) ;
      bson_append_oid( &obj1, "LobOid", &oid ) ;
      bson_append_long( &obj1, "LobSize", getSize ) ;
      bson_append_timestamp2( &obj1, "LobTimestamp", millis/1000, millis%1000 ) ;
      bson_append_string( &obj1, "LobMd5", wMd5 ) ;
      bson_append_finish_object( &obj1 ) ;
      rc = sdbInsert( cl, &obj1 ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      md5s[i] = wMd5 ;     // Store value of write large object file MD5

   }
   UINT32 j ;
   for( j = 0 ; j < writeLobNum ; ++j )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &( oids[j] ), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, rMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      ASSERT_EQ( SDB_OK, rc ) ;

      // Check read large object file is correct or not
      ASSERT_STREQ( md5s[j], rMd5 ) << "ERROR, Read wrong Lob data"
                                    << "\nWrite LOB MD5 = " << md5s[j]
                                    << "\nRead LOB MD5 = " << rMd5 ;
      rc = sdbRemoveLob( cl, &( oids[j] ) ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   // Clear the table[collection] in the end
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object seek read.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.005 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobSeekRead )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Seek_Read" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   UINT32 lobRead = 0 ;
   UINT32 seekSize = 1435 ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_oid_gen( &oid ) ;
   genLobData( lobBuffer, lobSize ) ;   // auto genatate data
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Seek Read Large Object
   memset( lobBuffer, 0, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Read LOB OVER>=*\n" ) ;
   // Check the write and read is correct or not
   SINT64 sizeGap = sizeWrite - seekSize ;
   ASSERT_EQ( sizeGap, lobRead ) << "Failed to seek read !\n"
                                 << "\nSize of Write = " << sizeWrite
                                 << "\nSize of Seek Read = " << lobSize ;
   printf( "*=<Correct LOB Write and LOB Seek Read>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test get information of large object. sdbGetLobSize()
*               { TestCase Number : LOB.ACCEPTANCE_TEST.006 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobGetLobSize )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Get_Lob_Size" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_oid_gen( &oid ) ;
   genLobData( lobBuffer, lobSize ) ;   // auto generate data
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Check the write and read is correct or not
   ASSERT_EQ( sizeWrite, strlen(lobBuffer) ) << "Failed to seek read !\n"
                                               << "\nSize of Write = "
                                               << sizeWrite
                                               << "\nSize of Seek Read = "
                                               << strlen(lobBuffer) ;
   printf( "*=<Correct LOB Write and LOB Seek Read>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test Open a exist large object and then Write data in.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.010 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobWriteExistOBJ )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Write_Exist_Object" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   bson_oid_t oid, oid1;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_oid_gen( &oid ) ;
   oid1 = oid ;
   genLobData( lobBuffer, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Checking exist LOB can be writed or not [Test Point]
   rc = sdbOpenLob( cl, &oid1, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   printf( "*=<Check Open Exist LOB OVER>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object basic operation, include open/write/read/seek/
*               getSize/remove/close
*               { TestCase Number : LOB.ACCEPTANCE_TEST.011 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobBasicOperation )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const UINT64 writeSize = lobSize ;
   const CHAR *prefixClName = "Lob_Basic_Operation" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid, rep_oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_oid_gen( &oid ) ;
   genLobData( lobBuffer, writeSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, writeSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read Large Object
   memset( lobBuffer, 0, lobSize ) ;
   UINT32 seekSize = 1435 ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Seek Read LOB OVER>=*\n" ) ;
   // Repeated Write Seek Read data to new Large Object
   memset( lobBuffer, 0, lobSize ) ;
   bson_oid_gen( &rep_oid ) ;
   rc = sdbOpenLob( cl, &rep_oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Repeated Write New LOB OVER>=*\n" ) ;
   // Get the Size of New Lob
   rc = sdbOpenLob( cl, &rep_oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 repSizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &repSizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Check the write and repeated write new LOB is correct or not
   SINT64 sizeGap = sizeWrite - repSizeWrite ;
   ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !"
                                  << "\nSize of Write = " << sizeWrite
                                  << "\nSize of Repeated Write = "
                                  << repSizeWrite ;
   printf( "*=<Correct LOB Write and Repeated Write New LOB>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbRemoveLob( cl, &rep_oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test Open the removed LOB is OK or not.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.012 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobReadOneLobAndRemove )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*300 ;
   const CHAR *prefixClName = "Lob__Read_One_Lob_And_Remove" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   const UINT32 cnt = 1000 ;
   UINT32 readLen = 0 ;
   bson_oid_t oid, oids[cnt] ;
   bson obj ;
   UINT32 i ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   for( i = 0 ; i < cnt ; ++i )
   {
      bson_oid_gen( &oid ) ;
      oids[i] = oid ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Read LOB ,Remove Lob, Read Lob
   for( i = 0 ; i < cnt ; ++i )
   {
      rc = sdbOpenLob( cl, &(oids[i]), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbRemoveLob( cl, &(oids[i]) ) ;// remove
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   printf( "*=<Read and Remove Lob Over>=*\n" ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : concurrent write different lob
*               { TestCase Number : LOB.ACCEPTANCE_TEST.009 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobConcurrentWrite )
{
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Concurrent_Write" ;
   const UINT64 lobSize = 1024*16 ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   const UINT32 putNum = 50 ;
   boost::thread *th[putNum] ;
   bson_oid_t oid ;

   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i] = new boost::thread( thrdWriteLob, prefixClName, lobBuffer, lobSize,
                                 &oid, &cs, clName ) ;
   }

   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i]->join() ;
      delete th[i] ;
   }
   printf( "*=<Concurrent Write LOB OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCS( cs ) ;
}

/*******************************************************************************
*@Description : Write LOB, then Read the writen Lob.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.009 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobConcurrentRead )
{
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Concurrent_Read" ;
   const UINT64 lobSize = 1024*16 ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   const UINT32 putNum = 20 ;
   boost::thread *th[putNum] ;
   bson_oid_t oids ;
   bson obj ;

   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   thrdWriteLob( prefixClName, lobBuffer, lobSize, &oids, &cs, clName ) ;   //write lob
/*
   bson_init( &obj ) ;
   bson_append_oid( &obj, "get_oid", &oids ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
*/

   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i] = new boost::thread( thrdReadLob, prefixClName, lobSize, oids ) ;
   }

   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i]->join() ;
      delete th[i] ;
   }
   printf( "*=<Concurrent Read LOB OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCS( cs ) ;
}

/*******************************************************************************
*@Description : Write LOB, then concurrent seek read lob
*               { TestCase Number : LOB.ACCEPTANCE_TEST.009 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobConcurrentSeekRead )
{
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Concurrent_Seek_Read" ;
   const UINT64 lobSize = 1024*16 ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   const UINT32 putNum = 20 ;
   SINT64 seekSz = 235 ;
   boost::thread *th[putNum] ;
   bson_oid_t oid, oids[putNum] ;

   // write lob
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      thrdWriteLob( prefixClName, lobBuffer, lobSize, &oid, &cs, clName ) ;
      oids[i] = oid ;
   }
   // concurrent seek read
   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i] = new boost::thread( thrdSeekReadLob, prefixClName, lobSize,
                                 oids[i], seekSz, SDB_LOB_SEEK_SET ) ;
   }

   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i]->join() ;
      delete th[i] ;
   }
   printf( "*=<Concurrent Seek Read LOB OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCS( cs ) ;
}

/*******************************************************************************
*@Description : Write LOB, then concurrent remove lob
*               { TestCase Number : LOB.ACCEPTANCE_TEST.009 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobConcurrentRemove )
{
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Concurrent_Remove" ;
   const UINT64 lobSize = 1024*16 ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 } ;
   const UINT32 putNum = 20 ;
   boost::thread *th[putNum] ;
   bson_oid_t oid, oids[putNum] ;

   // Write Large Object
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      thrdWriteLob( prefixClName, lobBuffer, lobSize, &oid, &cs, clName ) ;
      oids[i] = oid ;
   }
   // concurrent remove
   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i] = new boost::thread( thrdRemoveLob, prefixClName, oids[i] ) ;
   }

   for( UINT32 i = 0 ; i < putNum ; ++i )
   {
      th[i]->join() ;
      delete th[i] ;
   }
   printf( "*=<Concurrent Remove LOB OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCS( cs ) ;
}

/*******************************************************************************
*@Description : Get Create LOB Time.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.013 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobGetCreateTime )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 16*1024*1024 ;
   const CHAR *prefName = "LOB_Get_Create_Time" ;
   CHAR *wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[64] = { 0 } ;
   CHAR time[32] = { 0 } ;
   CHAR milsec[12] = { 0 } ;
   UINT64 millis = 0 ;
   INT32 getTime = 0 ;
   UINT32 readSize = 0 ;
   bson_oid_t oid ;
   bson obj ;
   bson_timestamp_t tm ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to create collection, rc = " << rc ;
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // Get System Current time
   struct timeval tv ;
   gettimeofday( &tv, NULL ) ;
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   genLobData( lobBuffer, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get LOB create time
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   //printf( "Get Lob Create Time, rc = %ld\n", millis ) ;
   UINT64 timer = tv.tv_sec * 1000 + tv.tv_usec / 1000 ;
   printf( "Get Sys Time = %ld\n Read Size =%ld\n", timer, millis ) ;
   ASSERT_LE( timer, millis ) << "Failed to Get Create LOB Time"
                              << "Get System Time = " << timer
                              << "Get LOB Create Time = " << millis ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object basic operation, include open/write/read/seek/
*               getSize/remove/close
*               { TestCase Number : LOB.ACCEPTANCE_TEST.011 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobLittleOperation )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   // 1b, 79b, 121b, 757b, 11k, 79k, 131k, 789k, 13M
   UINT64 lobSize[9] = { 1, 79, 121, 757, 11264, 80896, 134144, 807936,
                               13631488 } ;
   const CHAR *prefixClName = "Lob_little_Operation" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid, rep_oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   for( INT32 i = 0 ; i < 9 ; ++i )
   {
      const UINT64 writeSize = lobSize[i] ;
      // Write Large Object
      if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize[i], sizeof(char) ) ) )
      {
         perror( "lobBuffer" ) ;
         ASSERT_TRUE( false ) ;
      }
      bson_oid_gen( &oid ) ;
      genLobData( lobBuffer, writeSize ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, writeSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "*=<Write  %d LOB OVER>=*\n", i ) ;
      // Get the Size of Lob
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Read Large Object
      memset( lobBuffer, 0, lobSize[i] ) ;
      UINT32 seekSize = 0 ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize[i], lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "*=<Seek Read %d LOB OVER>=*\n", i ) ;
      // Repeated Write Seek Read data to new Large Object
      memset( lobBuffer, 0, lobSize[i] ) ;
      bson_oid_gen( &rep_oid ) ;
      rc = sdbOpenLob( cl, &rep_oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "*=<Repeated Write %d New LOB OVER>=*\n", i ) ;
      // Get the Size of New Lob
      rc = sdbOpenLob( cl, &rep_oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      SINT64 repSizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &repSizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Check the write and repeated write new LOB is correct or not
      SINT64 sizeGap = sizeWrite - repSizeWrite ;
      ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !"
                                     << "\nSize of Write = " << sizeWrite
                                     << "\nSize of Repeated Write = "
                                     << repSizeWrite ;
      printf( "*=<Correct %d LOB Write and Repeated Write New LOB>=*\n", i ) ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbRemoveLob( cl, &rep_oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
