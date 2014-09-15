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
#include <pthread.h>
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
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[64] = { 0 } ;
   UINT32 lobRead = 0 ;
   FILE *rFile ;
   bson_oid_t oid ;
   bson obj ;

   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   genLobData( lobWriteBuf, lobSize ) ;   // auto generate data
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = md5Code( lobWriteBuf, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Read Large Object
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
   rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   rc = md5Code( lobReadBuf, rMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from read
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
   printf( "*=<Read LOB OVER>=*\n" ) ;
   // Check the write and read is correct or not
   ASSERT_STREQ( wMd5, rMd5 ) << "*->ERROR, Read wrong data from LOB\n"
                              << "*->Write Lob Md5 = " << wMd5
                              << "\n*->Read  Lob Md5 =" << rMd5 ;
   printf( "*=<Correct LOB Write and LOB Read>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
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
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[64] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 writeLobNum = 20 ;
   bson_oid_t oid ;
   bson obj1 ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Allocate Memory
   if( NULL == (lobWriteBuf = (CHAR *)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Put 100 same Lob in SDB
   UINT32 i ;
   for( i = 0 ; i < writeLobNum ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      // Write large file to SDB
      genLobData( lobWriteBuf, lobSize ) ;   // Auto generate data
      bson_oid_gen( &oid ) ;
      if( 0 == i || (writeLobNum-1) == i )
      {
         bson_init( &obj1 ) ;
         bson_append_oid( &obj1, "$Oid", &oid ) ;
         bson_append_finish_object( &obj1 ) ;
         bson_print( &obj1 ) ;  // Print the oid
      }
      rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = md5Code( lobWriteBuf, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
      // Get the Size of Lob
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
      ASSERT_EQ( sizeWrite, lobSize ) << "ERROR, Write wrong data to Sdb"
                                      << "Put Lob Size = " << lobSize
                                      << "Get Lob Size = " << sizeWrite ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
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
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[64] = { 0 } ;
   UINT32 lobRead = 0 ;
   const UINT32 writeLobNum = 20 ;
   CHAR *md5s[writeLobNum] = { 0 } ;
   bson_oid_t oid, oids[writeLobNum] ;
   bson obj1 ;

   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Allocate Memory
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Put 100 same Lob in SDB
   UINT32 i ;
   for( i = 0 ; i < writeLobNum ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      // Write large file to SDB
      genLobData( lobWriteBuf, lobSize ) ;
      bson_oid_gen( &oid ) ;
      if( 0 == i || (writeLobNum-1) == i )
      {
         bson_init( &obj1 ) ;
         bson_append_oid( &obj1, "$Oid", &oid ) ;
         bson_append_finish_object( &obj1 ) ;
         bson_print( &obj1 ) ;  // Print the oid
      }
      oids[i] = oid ;
      rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
      // Get the Size of Lob
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
      ASSERT_EQ( sizeWrite, lobSize ) << "ERROR, Write wrong data to Sdb"
                                      << "Put Lob Size = " << lobSize
                                      << "Get Lob Size = " << sizeWrite ;
      rc = md5Code( lobWriteBuf, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get md5 from write lob, rc = " << rc ;
      md5s[i] = wMd5 ;     // Story value of write large object file MD5
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   // Allocate Memory
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   UINT32 j ;
   for( j = 0 ; j < writeLobNum ; ++j )
   {
      memset( lobReadBuf, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &( oids[j] ), readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
      rc = md5Code( lobReadBuf, rMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get md5 from read lob, rc = " << rc ;

      // Check read large object file is correct or not
      ASSERT_STREQ( md5s[j], rMd5 ) << "ERROR, Read wrong Lob data"
                                    << "\nWrite LOB MD5 = " << md5s[j]
                                    << "\nRead LOB MD5 = " << rMd5 ;

      rc = sdbRemoveLob( cl, &( oids[j] ) ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
   }
   // Clear the table[collection] in the end
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
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
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[128] = { 0 } ;
   UINT32 lobRead = 0 ;
   UINT32 seekSize = 1435 ;
   bson_oid_t oid ;
   bson obj ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   genLobData( lobWriteBuf, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = md5Code( lobWriteBuf, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Read Large Object
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
   rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
   rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   printf( "*=<Read LOB OVER>=*\n" ) ;
   // Check the write and read is correct or not
   SINT64 sizeGap = sizeWrite - seekSize ;
   ASSERT_EQ( sizeGap, lobRead ) << "Failed to seek read !\n"
                                 << "\nSize of Write = " << sizeWrite
                                 << "\nSize of Seek Read = " << strlen(lobReadBuf) ;
   printf( "*=<Correct LOB Write and LOB Seek Read>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove Lob, rc = " << rc ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
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
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[128] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;

   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   genLobData( lobWriteBuf, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   // Check the write and read is correct or not
   ASSERT_EQ( sizeWrite, strlen(lobWriteBuf) ) << "Failed to seek read !\n"
                                               << "\nSize of Write = "
                                               << sizeWrite
                                               << "\nSize of Seek Read = "
                                               << strlen(lobWriteBuf) ;
   printf( "*=<Correct LOB Write and LOB Seek Read>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
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
   const CHAR *fileName ="/home/users/huxiaojun/sequoiadb/"
                         "testcases/hlt/drivers_testcases/"
                         "C/LOB_GTestCase/srcLob.json" ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[128] = { 0 } ;
   bson_oid_t oid, oid1;
   bson obj ;

   if( NULL == (lobWriteBuf = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   oid1 = oid ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   genLobData( lobWriteBuf, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Checking exist LOB can be writed or not [Test Point]
   rc = sdbOpenLob( cl, &oid1, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open exist LOB in write\n"
                               << ", rc = " << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
   ASSERT_TRUE( SDB_OK != rc ) << "ERROR, Write exist LOB, rc = " << rc ;
   printf( "*=<Check Open Exist LOB OVER>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove Lob, rc = " << rc ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
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
   const UINT64 writeSize = lobSize - 1 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[128] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid, rep_oid ;
   bson obj ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   genLobData( lobWriteBuf, writeSize ) ;
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = md5Code( lobWriteBuf, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, writeSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Read Large Object
   UINT32 seekSize = 1435 ;
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
   rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
   rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   printf( "*=<Seek Read LOB OVER>=*\n" ) ;
   // Repeated Use [lobWriteBuf]
   memset( lobWriteBuf, 0, lobSize ) ;
   strncpy( lobWriteBuf, lobReadBuf, lobRead ) ;  // Why will Core Dump
   // Repeated Write Seek Read data to new Large Object
   bson_oid_gen( &rep_oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Rep_Oid", &rep_oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbOpenLob( cl, &rep_oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Repeated Write New LOB OVER>=*\n" ) ;
   // Get the Size of New Lob
   rc = sdbOpenLob( cl, &rep_oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
   SINT64 repSizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &repSizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   // Check the write and repeated write new LOB is correct or not
   SINT64 sizeGap = sizeWrite - repSizeWrite ;
   ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !"
                                  << "\nSize of Write = " << sizeWrite
                                  << "\nSize of Repeated Write = "
                                  << repSizeWrite ;
   printf( "*=<Correct LOB Write and Repeated Write New LOB>=*\n" ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
   rc = sdbRemoveLob( cl, &rep_oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;

   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;

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
TEST( acceptanceTest, lobRemoveThenOpen )
{
/*
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*300 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[128] = { 0 } ;
   const UINT32 cnt = 1000 ;
   UINT32 readLen = 0 ;
   bson_oid_t oid, oids[cnt] ;
   bson obj ;
   UINT32 i ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   for( i = 0 ; i < cnt ; ++i )
   {
      // Write Large Object
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 1 == i || cnt-1 == i )
         bson_print( &obj ) ;  // Print the oid
      oids[i] = oid ;
      genLobData( lobWriteBuf, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
      //printf( "*=<Write LOB OVER>=*\n" ) ;
   }
   // Remove One Lob
   rc = sdbRemoveLob( cl, &oids[500] ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove Lob, rc = " << rc ;
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Checking exist LOB can be writed or not [Test Point]
   for( i = 0 ; i < cnt ; ++i )
   {
      if( 500 != i )
      {
         rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
         ASSERT_EQ( SDB_OK, rc ) << "ERROR, Open a removed LOB sucess\n"
                                     << ", rc = " << rc ;
         rc = sdbReadLob( lob, lobSize, lobReadBuf, &readLen ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to Read LOB, rc = " << rc ;
         rc = sdbCloseLob( &lob ) ;
         ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in Read, rc = " << rc ;
      }
   }
   printf( "*=<Check Open Removed Lob Over>=*\n" ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
*/
}

/*******************************************************************************
*@Description : While write LOB, Read the Lob.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.009 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobConcurrentWriteRead )
{
   const UINT64 lobSize = 1024*1024*16 ;
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *fileName ="/home/users/huxiaojun/sequoiadb/"
                         "testcases/hlt/drivers_testcases/"
                         "C/LOB_GTestCase/srcLob.json" ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[128] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid ;
   bson obj ;
   pthread_t wlobTid, rlobTid;
   struct Lob wlobarg ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   // Initalize thread arguement
   genLobData( lobWriteBuf, lobSize ) ;
   wlobarg.CL = cl ;
   wlobarg.OID = oid ;
   wlobarg.Buffer = lobWriteBuf ;
   rc = pthread_create( &wlobTid, NULL, tWriteLob, (void *)&wlobarg ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed create thread for write LOB,"
                           << " rc =" << rc ;
   usleep( 5000 ) ;   // delay 5000 millisecond
   // Read Large Object
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   struct Lob rlobarg ;
   rlobarg.CL = cl ;
   rlobarg.OID = oid ;
   rlobarg.Buffer = lobReadBuf ;
   rc = pthread_create( &rlobTid, NULL,tReadLob, (void *)&rlobarg ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed create thread for read LOB,"
                               << " rc =" << rc ;
   pthread_join( wlobTid, NULL ) ;
   pthread_join( rlobTid, NULL ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
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
   const CHAR *createTime = "date +%s > time.txt" ;
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

   rc = tCreateCollection( &db, &cs, prefName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to create collection, rc = " << rc ;
   if( NULL == ( lobBuffer = (CHAR *)malloc(lobSize) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
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
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Open LOB in write, rc = " << rc ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in Write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get LOB create time
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in Read, rc = " << rc ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Get LOB create time, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB, rc = " << rc ;
   //printf( "Get Lob Create Time, rc = %ld\n", millis ) ;
   UINT64 timer = tv.tv_sec * 1000 + tv.tv_usec / 1000 ;
   printf( "Get Sys Time = %ld\n Read Size =%ld\n", timer, millis ) ;
   ASSERT_LE( timer, millis ) << "Failed to Get Create LOB Time"
                              << "Get System Time = " << timer
                              << "Get LOB Create Time = " << millis ;
}
