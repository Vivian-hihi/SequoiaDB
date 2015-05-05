/*******************************************************************************
*@Description : All LOB Abnormal Testcase.Description of testcase
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
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <unistd.h>
#include "testcommon.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
// write lob1
void splitLobWrite1( sdbCollectionHandle cl, CHAR *lobBuffer,
                    const UINT64 lobSize, UINT64 putNum, bson_oid_t oids[] )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   INT32 ret = SDB_OK ;
   bson_oid_t oid ;
   UINT32 i, j ;
   bson obj ;

   bson_oid_gen( &oid ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "begin to write lob \n" ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      if( SDB_OK != rc && SDB_DMS_NOTEXIST != rc &&
          SDB_RTN_CONTEXT_NOTEXIST != rc )
      {
         ASSERT_TRUE( false ) << "rc : " << rc ;
         ret = rc ;
      }
   }
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   oids[0] = oid ;
   if( 0 != ret )
      printf( "throw out in %d, rc = %d\n", i, ret ) ;
   printf( "write lob over\n" ) ;
}

// read lob
void splitLobRead1( sdbCollectionHandle cl, CHAR *lobBuffer,
                   const UINT64 lobSize, UINT64 putNum, bson_oid_t *oids,
                   CHAR md5[] )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   UINT32 readLen = 0 ;
   CHAR rmd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;
   UINT32 i = 0 ;
   INT32 ret = SDB_OK ;

   oid = *(oids+i) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   UINT64 readSz = 0 ;
   printf( "begin to read lob \n" ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
      if( SDB_OK != rc && SDB_DMS_NOTEXIST != rc &&
          SDB_RTN_CONTEXT_NOTEXIST != rc && SDB_LOB_SEQUENCE_NOT_EXIST != rc )
      {
         ASSERT_TRUE( false ) << "rc : " << rc ;
         ret = rc ;
      }
   }
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   //ASSERT_STREQ( md5, rmd5 ) ;   // compare md5
   UINT64 size = lobSize*putNum ;
   if( 0 != ret )
      printf( "throw out in %d, rc = %d\n", i, ret ) ;
   printf( "read lob over\n" ) ;
}

// drop collection space and collection
void dropCSandCL( const CHAR *clName )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;

   rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbDropCollectionSpace( db, COMMCSNAME ) ;
   ASSERT_EQ( SDB_LOCK_FAILED, rc ) ;
   rc = sdbGetCollectionSpace( db, COMMCSNAME, &cs ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "drop cs and cl over\n" ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test Open and Close not exist LOB.
*               { TestCase Number : LOB.ABNORMAL_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, NotExistLob )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbLobHandle lob1 = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Open_Not_Exist_Lob" ;
   const UINT64 lobSize = 1024*1024*16 ;
   CHAR clName[70] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
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
   // Drop CL
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Create CL beginning
   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read not exist lob [TEST POINT]
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   UINT64 millis = 0 ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   rc = sdbCloseLob( &lob ) ;    // lob handle don't exist
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   rc = sdbRemoveLob( lob, &oid ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test read unfinished created Lob
*               { TestCase Number : LOB.ABNORMAL_TEST.002 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, lobReadUnfinishedWriteLob )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbLobHandle lob1 = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Read_Unfinished_Write_Lob" ;
   CHAR clName[70] = { 0 } ;
   const UINT32 lobSize = 12*1024 ;
   CHAR *lobBuffer = NULL ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   memset( lobBuffer, 0, lobSize ) ;
   genLobData( lobBuffer, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( SDB_LOB_IS_NOT_AVAILABLE != rc )
   {
      rc = sdbInsert( cl, &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   // lob creation is not completed, then read lob [ TEST POINT ]
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_LOB_IS_NOT_AVAILABLE, rc ) ;
   UINT32 readLen = 0 ;

   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_LOB_IS_NOT_AVAILABLE, rc ) ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test drop collection and then create same collection, inpect the
*               lob is exist or not.

*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, lobWriteLobDropCLThenCreateCL )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR* preName = "DropCL_Then_CreateCL_Inspect_LOB" ;
   CHAR clName[70] = { 0 } ;
   CHAR lobBuffer[1024] = { 0 } ;
   UINT32 lobSize = 1023 ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   bson_oid_t oid ;
   bson obj ;

   // Write Lob
   printf( "start to write lob\n" ) ;
   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   genLobData( lobBuffer, lobSize ) ;
   bson_oid_gen( &oid ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "write lob over\n" ) ;
   // Get Lob size and put in normal record
/*
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   printf( "write lob over2\n" ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "get lobsize1 over\n" ) ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "get lobsize2 over\n" ) ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "get lobsize3 over\n" ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "get lobsize over\n" ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "Lob_Oid", &oid ) ;
   bson_append_long( &obj, "Lob_Size", getSize ) ;
   bson_append_timestamp2( &obj, "Lob_Timestamp", millis/1000, millis%1000 ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   rc = sdbInsert( cl, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
*/
   // Drop collecrtion
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<drop collection success>=*\n" ) ;
   // Create collection again
   db = SDB_INVALID_HANDLE ;
   cs = SDB_INVALID_HANDLE ;
   cl = SDB_INVALID_HANDLE ;
   lob = SDB_INVALID_HANDLE ;
   getSize = 0 ;
   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read lob and check the lob is exist or not [ TEST POINT ]
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   printf( "get lob size = %lld\n", getSize ) ;

   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Create Same lob and read lob from other table
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, diffCLReadSameLob )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR* preName = "Diff_CL_Read_Same_Lob" ;
   const CHAR* preName2 = "Diff_CL_Read_Same_Lob2" ;
   CHAR clName[70] = { 0 } ;
   CHAR *lobBuffer = { 0 } ;
   UINT32 lobSize = 20*1024*1024 ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   UINT32 readLen = 0 ;
   bson_oid_t oid ;
   bson obj ;

   // Write Lob
   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "malloc_lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   bson_oid_gen( &oid ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Get Lob size and put in normal record
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "Lob_Oid", &oid ) ;
   bson_append_long( &obj, "Lob_Size", getSize ) ;
   bson_append_timestamp2( &obj, "Lob_Timestamp", millis/1000, millis%1000 ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   rc = sdbInsert( cl, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Create new collection in same collection space and read lob[ TEST POINT ]
   rc = lobCreateCollection( &db, &cs, preName2, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "create collection success : %s\n", clName ) ;
   lob = SDB_INVALID_HANDLE ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
   ASSERT_NE( SDB_OK, rc ) ;
   //rc = sdbCloseLob( &lob ) ;
   //ASSERT_EQ( SDB_OK, rc ) ;
   printf( "other collection read : %d\n", readLen ) ;
   // Create different collection space and same collectin, read lob[TEST POINT]
   rc = lobCreateCollection( &db, &cs, preName, &cl, clName, COMMCSNAME1 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "create collection success : %s\n", clName ) ;
   lob = SDB_INVALID_HANDLE ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
   ASSERT_NE( SDB_OK, rc ) ;

   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   // Drop collecrtion
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : create collection space specify abnormal test. specify size not
*               in { 0, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288}
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, lobSpecifyLobPageSizeAbnormal )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbConnectionHandle db1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cs1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cl1 = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefName = "Specify_Lob_Page_Size_Abnormal" ;
   CHAR clName[128] = { 0 } ;
   INT32 pageSz[12] = { -4096, -20, 100, 4095, 8190, 16383, 32765, 65537, 131073,
                        262149, 524281, 699990 } ;
   bson lobPzObj ;
   INT32 i ;

   for( i = 0 ; i < 12 ; ++i )
   {
      bson_init( &lobPzObj ) ;
      bson_append_int( &lobPzObj, "LobPageSize", pageSz[i] ) ;
      bson_append_finish_object( &lobPzObj ) ;
      bson_print( &lobPzObj ) ;
      rc = lobCreateCollectionPz( &db, &cs, prefName, &cl, clName, &lobPzObj ) ;
      ASSERT_EQ( SDB_INVALIDARG, rc ) ;

   }
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : When write Lob, we drop collection space.
*               When write lob, can drop CL, cannot drop CS.
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, dropCSwhenWriteLob )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefName = "DropCS_When_Write_Lob" ;
   CHAR clName[128] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   UINT64 putNum = 20 ;
   const UINT64 lobSize = 20*1024*1024 ;
   boost::thread *th1, *th2 ;
   bson_oid_t oids[putNum] ;
   INT32 i ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   // write Lob
   th1 = new boost::thread( splitLobWrite1, cl, lobBuffer, lobSize, putNum, oids ) ;
   //boost::this_thread::sleep( boost::posix_time::milliseconds(3000) ) ;
   //printf( "sleep 3000 milliseconds\n" ) ;
   th2 = new boost::thread( dropCSandCL, clName ) ;
   th2->join() ;
   th1->join() ;

   delete th1 ;
   delete th2 ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : When read Lob, we drop collection space and collection.
*               When read lob, can drop CL, cannot drop CS.
*@Modify List :
*               2014-10-22   xiaojun Hu   Init
*******************************************************************************/
TEST( abnormalTest, dropCSwhenReadLob )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefName = "DropCS_When_Write_Lob" ;
   CHAR clName[128] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   UINT64 putNum = 50 ;
   const UINT64 lobSize = 20*1024*1024 ;
   CHAR md5[30] = { 0 } ;
   boost::thread *th1, *th2 ;
   bson_oid_t oids[putNum] ;
   INT32 i ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   splitLobWrite1( cl, lobBuffer, lobSize, putNum, oids ) ;  // write one lob
   // read thread and drop
   th1 = new boost::thread( splitLobRead1, cl, lobBuffer, lobSize, putNum, oids, md5 ) ;
   //boost::this_thread::sleep( boost::posix_time::milliseconds(3000) ) ;
   //printf( "sleep 3000 milliseconds\n" ) ;
   th2 = new boost::thread( dropCSandCL, clName ) ;
   th2->join() ;
   th1->join() ;

   delete th1 ;
   delete th2 ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
