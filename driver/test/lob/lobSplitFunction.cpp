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
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <unistd.h>
#include "testcommon.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )

// get Group Name that collectin located in
/*
INT32 lobGetCLrgName( sdbConnectionHandle db, const CHAR *clName,
                      CHAR srcGroup[] )
{
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   bson condObj, selectObj, rgObj ;
   bson_iterator it, subit ;
   bson_type type ;
   const CHAR *csRgName = NULL ;

   // get cs group
   bson_init( &condObj ) ;
   bson_init( &selectObj ) ;
   bson_append_string( &condObj, "Name", clName ) ;
   bson_append_int( &selectObj, "Details", 1 ) ;
   bson_append_finish_object( &condObj ) ;
   bson_append_finish_object( &selectObj ) ;
   rc = sdbGetSnapshot( db, SDB_SNAP_COLLECTIONS, &condObj,
                        &selectObj, NULL, &cursor ) ;
   CHECK_RC_CODE( rc, "failed to get snapshot for collection" ) ;
   //rc = sdbGetSnapshot( db, SDB_SNAP_COLLECTIONS, NULL, NULL, NULL, &cursor ) ;
   bson_init( &rgObj ) ;
   rc = sdbCurrent( cursor, &rgObj ) ;
   //bson_print( &rgObj ) ;
   bson_iterator_init( &it, &rgObj ) ;
   bson_iterator_subiterator( &it, &subit ) ;
   while( bson_iterator_more( &subit ) )
   {
      if( BSON_EOO != bson_iterator_next( &subit ) )
      {
         bson subObj ;
         bson_init( &subObj ) ;
         //printf( "Key : %s\n", bson_iterator_key( subit ) ) ;
         bson_iterator_subobject( &subit, &subObj ) ;
         if( bson_find( &subit, &subObj, "GroupName" ) )
         {
            csRgName = bson_iterator_string( &subit ) ;
         }
         bson_destroy( &subObj ) ;
      }
   }
   memcpy( srcGroup, csRgName, strlen(csRgName) ) ;
   bson_destroy( &condObj ) ;
   bson_destroy( &selectObj ) ;
   bson_destroy( &rgObj ) ;
   sdbReleaseCursor( cursor ) ;
done:
   return rc ;
error:
   goto done ;
}

// Get Group Name that collection will be split across
INT32 lobGetRgName( sdbConnectionHandle db, const CHAR *clName,
                    CHAR dstGroup[] )
{
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   bson glselObj, lrgObj ;
   bson_iterator it ;
   const CHAR *rgName = NULL ;
   CHAR srcGroup[30] = { 0 } ;

   // get all group
   bson_init( &glselObj ) ;
   bson_append_int( &glselObj, "GroupName", 1 ) ;
   bson_append_finish_object( &glselObj ) ;
   rc = sdbGetList( db, SDB_LIST_GROUPS, NULL, &glselObj, NULL, &cursor ) ;
   CHECK_RC_CODE( rc, "failed to get groups" ) ;
   rc = lobGetCLrgName( db, clName, srcGroup ) ;
   CHECK_RC_CODE( rc, "failed to get source group" ) ;
   bson_init( &lrgObj ) ;
   while( SDB_OK == rc )
   {
      rc = sdbNext( cursor, &lrgObj ) ;
      bson_iterator_init( &it, &lrgObj ) ;
      rgName = bson_iterator_string( &it ) ;
      if( 0 != strcmp( "SYSCatalogGroup", rgName ) &&
          0 != strcmp( rgName, srcGroup ) )
      {
         break ;
      }
   }
   memcpy( dstGroup, rgName, strlen(rgName) ) ;
   bson_destroy( &lrgObj ) ;
   bson_destroy( &glselObj ) ;
done:
   return rc ;
error:
   goto done ;
}
*/

// split
void lobSplitCollection( sdbCollectionHandle cl,
                         const CHAR *srcGroup, const CHAR *dstGroup,
                         const bson *beginCond, const bson *endCond )
{
   INT32 rc = SDB_OK ;

   // split collection
   printf( "source group = %s, destination group = %s\n", srcGroup, dstGroup ) ;
   rc = sdbSplitCollection( cl, srcGroup, dstGroup, beginCond, endCond ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "split data successful\n" ) ;
}

void lobSplitAsyncCollection( sdbCollectionHandle cl,
                              const CHAR *srcGroup, const CHAR *dstGroup,
                              const bson *beginCond, const bson *endCond )
{
   INT32 rc = SDB_OK ;
   SINT64 taskID = 0 ;

   // split collection
   printf( "source group = %s, destination group = %s\n", srcGroup, dstGroup ) ;
   rc = sdbSplitCLAsync( cl, srcGroup, dstGroup, beginCond, endCond, &taskID ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "split data successful\n" ) ;
}

/*******************************************************************************
*@Description : Write plenty of large object to SDB_OK and then read this
*               object,Testing Read operation is available or not.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.003 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( splitAboutLob, lobHashSplitThenWrite )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefName = "Lob_Hash_Split_Then_Write" ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   UINT32 i = 0 ;
   //UINT64 putNum = 10 ;
   UINT64 putNum = 3 ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oids[putNum] ;
   bson obj, rgobj ;
   const CHAR *rgName ;
   bson startObj, endObj ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // hash split
   if( NULL == ( lobBuffer = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 300 ) ;
   bson_append_int( &endObj, "Partition", 700 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;
   // must be CL name like : foo.bar
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
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   // write lob
   splitLobWrite( cl, lobBuffer, lobSize, putNum, oids ) ;
   printf( "write lob successful\n" ) ;
   // read lob
   splitLobRead( cl, lobBuffer, lobSize, putNum, oids, md5 ) ;
   printf( "read lob sucessful\n" ) ;
   // remove lob
   splitLobRemove( cl, lobBuffer, lobSize, putNum, oids ) ;
   printf( "remove lob sucessful\n" ) ;
   // drop cl
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Write Large object to sdb, then split, read and remove
*               { TestCase Number : LOB.ACCEPTANCE_TEST.003 }
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( splitAboutLob, lobWriteThenHashSplit )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefName = "Lob_Write_Then_Hash_Split" ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   UINT32 i = 0 ;
   //UINT64 putNum = 10 ;
   UINT64 putNum = 3 ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oids[putNum] ;
   bson obj, rgobj ;
   bson_iterator it[1] ;
   const CHAR *rgName ;
   bson startObj, endObj ;

   if( NULL == ( lobBuffer = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // write lob
   splitLobWrite( cl, lobBuffer, lobSize, putNum, oids ) ;
   printf( "write lob successful\n" ) ;
   // hash split
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 300 ) ;
   bson_append_int( &endObj, "Partition", 700 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;
   // must be CL name like : foo.bar
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "=>get destGroup = %s, =>get srcGroup = %s\n", srcGroup, dstGroup ) ;
      rc = sdbSplitCollection( cl, srcGroup, dstGroup , &startObj, &endObj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "split data successful\n" ) ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   // read lob
   splitLobRead( cl, lobBuffer, lobSize, putNum, oids, md5 ) ;
   printf( "read lob sucessful\n" ) ;
   // remove lob
   splitLobRemove( cl, lobBuffer, lobSize, putNum, oids ) ;
   printf( "remove lob sucessful\n" ) ;
   // drop cl
   //rc = sdbDropCollection( cs, clName ) ;
   //ASSERT_EQ( SDB_OK, rc ) ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Range split collection, and then writen lob will throw error[-6].
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( splitAboutLob, lobRangeSplitThenWrite )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefName = "Lob_Range_Split_Write" ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   bson_oid_t oid ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   bson startObj, endObj ;

   rc = lobCreateCollection1( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // range split
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "no", 10 ) ;
   bson_append_int( &endObj, "no", 10000 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;
   // must be CL name like : foo.bar
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "=>get destGroup = %s, =>get srcGroup = %s\n", srcGroup, dstGroup ) ;
      rc = sdbSplitCollection( cl, srcGroup, dstGroup , &startObj, &endObj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // write lob [TestPoint]
      bson_oid_gen( &oid ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_INVALIDARG, rc ) ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   // clean in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Concurrent Write Lob and Split
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
void conWriteLobAndSplit()
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbConnectionHandle db1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cs1 = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl1 = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*16 ;
   //const UINT64 lobSize = 1024*1024*16 ;
   //Lob_Write_Then_Range_Split
   const CHAR *prefName = "Lob_Concurrent_Write_And_Split" ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   //UINT64 putNum = 50 ;    // more than once and let error out
   UINT64 putNum = 50 ;    // more than once and let error out
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oids[putNum] ;
   bson startObj, endObj ;
   boost::thread *thSp, *thWLob ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = lobCreateCollection( &db1, &cs1, prefName, &cl1, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // range split
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   rc = md5Code( lobBuffer, md5, ENCRYTED_STR_LEN ) ;   // get md5 code
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 20 ) ;
   bson_append_int( &endObj, "Partition", 500 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;
   // must be CL name like : foo.bar
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )  //inspect standalone or other
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      printf( "cs name : %s\n", _clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // thread run
      //printf( "source group = %s, destination group = %s\n", srcGroup, dstGroup ) ;
      thWLob = new boost::thread( splitLobWrite, cl , lobBuffer, lobSize,
                                  putNum, oids ) ;
      boost::this_thread::sleep( boost::posix_time::milliseconds(3000) ) ;
      thSp = new boost::thread( lobSplitCollection, cl1, srcGroup, dstGroup,
                                &startObj, &endObj ) ;
      thSp->join() ;
      thWLob->join() ;
      // read lob and inspect md5
      //splitLobRead( cl, lobBuffer, lobSize, putNum, oids, md5 ) ;

      delete thSp ;
      delete thWLob ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   // clean in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "<<<finist remove collection>>>\n" ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
// do 20 times
TEST( splitAboutLob, lobConcurrentWriteAndSplit )
{
   INT32 num = 10 ;
   INT32 i ;

   for( i = 0 ; i < num ; ++i )
   {
      printf( "=====>loop times : %d\n", i ) ;
      conWriteLobAndSplit() ;
   }
   printf( "=====>total loop %d times\n", num ) ;
}

/*******************************************************************************
*@Description : Concurrent Read Lob and Split
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( splitAboutLob, lobConcurrentReadAndSplit )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbConnectionHandle db1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cs1 = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl1 = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefName = "Lob_Concurrent_Read_And_Split" ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   //UINT64 putNum = 20 ;
   UINT64 putNum = 3 ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oids[putNum] ;
   bson startObj, endObj ;
   boost::thread *thSp, *thRLob ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = lobCreateCollection( &db1, &cs1, prefName, &cl1, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // range split
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   rc = md5Code( lobBuffer, md5, ENCRYTED_STR_LEN ) ;   // get md5 code
   printf( "md5 : %s\n", md5 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 20 ) ;
   bson_append_int( &endObj, "Partition", 500 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;

   // Write Lob
   splitLobWrite( cl, lobBuffer, lobSize, putNum, oids ) ;
   // must be CL name like : foo.bar
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // thread run
      //printf( "source group = %s, destination group = %s\n", srcGroup, dstGroup ) ;
      thSp = new boost::thread( lobSplitCollection, cl1, srcGroup, dstGroup,
                                &startObj, &endObj ) ;
      //boost::this_thread::sleep( boost::posix_time::milliseconds(1000) ) ;
      thRLob = new boost::thread( splitLobRead, cl, lobBuffer, lobSize,
                                  putNum, oids, md5 ) ;
      //boost::this_thread::sleep( boost::posix_time::milliseconds(3000) ) ;
      thSp->join() ;
      thRLob->join() ;

      // clean in the end
      rc = sdbDropCollection( cs, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      delete thSp ;
      delete thRLob ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Concurrent Remove Lob and Split
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( splitAboutLob, lobConcurrentRemoveAndSplit )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbConnectionHandle db1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cs1 = SDB_INVALID_HANDLE ;
   sdbCSHandle cl1 = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefName = "Lob_Concurrent_Remove_And_Split" ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   //UINT64 putNum = 20 ;
   UINT64 putNum = 3 ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oids[putNum] ;
   bson startObj, endObj ;
   boost::thread *thSp, *thRmLob ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = lobCreateCollection( &db1, &cs1, prefName, &cl1, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // range split
   if( NULL == ( lobBuffer = (CHAR *)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   genLobData( lobBuffer, lobSize ) ;
   rc = md5Code( lobBuffer, md5, ENCRYTED_STR_LEN ) ;   // get md5 code
   printf( "md5 : %s\n", md5 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 20 ) ;
   bson_append_int( &endObj, "Partition", 500 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;

   // Write Lob
   splitLobWrite( cl, lobBuffer, lobSize, putNum, oids ) ;
   // must be CL name like : foo.bar
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // thread run
      //printf( "source group = %s, destination group = %s\n", srcGroup, dstGroup ) ;
      thSp = new boost::thread( lobSplitCollection, cl1, srcGroup, dstGroup,
                                &startObj, &endObj ) ;
      boost::this_thread::sleep( boost::posix_time::milliseconds(1000) ) ;
      thRmLob = new boost::thread( splitLobRemove, cl, lobBuffer, lobSize,
                                   putNum, oids ) ;
      //boost::this_thread::sleep( boost::posix_time::milliseconds(3000) ) ;
      thSp->join() ;
      thRmLob->join() ;

      // clean in the end
      rc = sdbDropCollection( cs, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      delete thSp ;
      delete thRmLob ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Write plenty of large object to SDB_OK and then read this
*               object,Testing Read operation is available or not.
*               { TestCase Number : LOB.ACCEPTANCE_TEST.003 }
*               need add : 1.async split write/read/remove/concurrent
*                          2.async splitCollectionByPercent/SplitCLbyPercentAsyns
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/
TEST( splitAboutLob, lobHashSplitAsyncThenWrite )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefName = "Lob_Hash_Split_Async_Then_Write" ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
   UINT32 i = 0 ;
   //UINT64 putNum = 10 ;
   UINT64 putNum = 3 ;
   CHAR dstGroup[30] = { 0 } ;
   CHAR srcGroup[30] = { 0 } ;
   CHAR md5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   bson_oid_t oids[putNum] ;
   bson obj, rgobj ;
   const CHAR *rgName ;
   bson startObj, endObj ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // hash split
   if( NULL == ( lobBuffer = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_init( &startObj ) ;
   bson_init( &endObj ) ;
   bson_append_int( &startObj, "Partition", 300 ) ;
   bson_append_int( &endObj, "Partition", 700 ) ;
   bson_append_finish_object( &startObj ) ;
   bson_print( &startObj ) ;
   bson_append_finish_object( &endObj ) ;
   bson_print( &endObj ) ;
   // must be CL name like : foo.bar
   rc = sdbListReplicaGroups( db, &cursor ) ;
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = lobGetCLrgName( db, _clName, srcGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = lobGetRgName( db, _clName, dstGroup ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "=>get destGroup = %s, =>get srcGroup = %s\n", srcGroup, dstGroup ) ;
      lobSplitAsyncCollection ( cl, srcGroup, dstGroup, &startObj, &endObj ) ;
   }
   else if( SDB_RTN_COORD_ONLY == rc )
   {
      printf( "run mode is standalone\n" ) ;
   }
   else
   {
      ASSERT_TRUE( false ) << "rc : " << rc ;
   }
   // write lob
   splitLobWrite( cl, lobBuffer, lobSize, putNum, oids ) ;
   printf( "write lob successful\n" ) ;
   // read lob
   splitLobRead( cl, lobBuffer, lobSize, putNum, oids, md5 ) ;
   printf( "read lob sucessful\n" ) ;
   // remove lob
   splitLobRemove( cl, lobBuffer, lobSize, putNum, oids ) ;
   printf( "remove lob sucessful\n" ) ;
   // drop cl
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_destroy( &startObj ) ;
   bson_destroy( &endObj ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
