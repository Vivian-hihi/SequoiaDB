/*******************************************************************************
*@Description : Just one testcase from Other tests.
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <gtest/gtest.h>
#include <malloc.h>
#include <unistd.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include "testcommon.h"
#include "client.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
boost::mutex mu ;
// get Group Name that collectin located in
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

void conWriteLobAndSplit()
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbCSHandle cl1 = SDB_INVALID_HANDLE ;
   sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   //Lob_Write_Then_Range_Split
   const CHAR *prefName = "Lob_Concurrent_Write_And_Split" ;
   CHAR clName[128] = { 0 } ;
   CHAR _clName[128] = { 0 } ;
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
   if( SDB_OK == rc )
   {
      sprintf( _clName, "%s%s%s", COMMCSNAME, ".", clName ) ;
      rc = sdbGetCollection( db, _clName, &cl1 ) ;   // get collection
      ASSERT_EQ( SDB_OK, rc ) ;
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

      // clean in the end
      //rc = sdbDropCollection( cs, clName ) ;
      //ASSERT_EQ( SDB_OK, rc ) ;
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
   INT32 num = 20 ;
   INT32 i ;

   for( i = 0 ; i < num ; ++i )
   {
      printf( "=====>loop times : %d\n", i ) ;
      conWriteLobAndSplit() ;
   }
   printf( "=====>total loop %d times\n", num ) ;
}




































































/*
INT32 lobGetCLrgName( sdbConnectionHandle db, const CHAR *clName,
                      CHAR srcGroup[] )
{
   sdbCursorHandle cur = SDB_INVALID_HANDLE ;
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
                        &selectObj, NULL, &cur ) ;
   CHECK_RC_CODE( rc, "failed to get snapshot for collection" ) ;
   //rc = sdbGetSnapshot( db, SDB_SNAP_COLLECTIONS, NULL, NULL, NULL, &cur ) ;
   bson_init( &rgObj ) ;
   rc = sdbCurrent( cur, &rgObj ) ;
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
   sdbReleaseCursor( cur ) ;
done:
   return rc ;
error:
   goto done ;
}

// Get Group Name that collection will be split across
INT32 lobGetRgName( sdbConnectionHandle db, const CHAR *clName,
                    CHAR dstGroup[] )
{
   sdbCursorHandle cur = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   bson glselObj, lrgObj ;
   bson_iterator it ;
   const CHAR *rgName = NULL ;
   CHAR srcGroup[30] = { 0 } ;

   // get all group
   bson_init( &glselObj ) ;
   bson_append_int( &glselObj, "GroupName", 1 ) ;
   bson_append_finish_object( &glselObj ) ;
   rc = sdbGetList( db, SDB_LIST_GROUPS, NULL, &glselObj, NULL, &cur ) ;
   CHECK_RC_CODE( rc, "failed to get groups" ) ;
   rc = lobGetCLrgName( db, clName, srcGroup ) ;
   CHECK_RC_CODE( rc, "failed to get source group" ) ;
   bson_init( &lrgObj ) ;
   while( SDB_OK == rc )
   {
      rc = sdbNext( cur, &lrgObj ) ;
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

// write lob
/*
void splitLobWrite( sdbCollectionHandle cl, CHAR *lobBuffer,
                    const UINT64 lobSize, UINT64 putNum, bson_oid_t oids[] )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   bson_oid_t oid ;
   UINT32 i, j ;
   bson obj ;

   for( i = 0 ; i < putNum ; ++i )
   {
      bson_oid_gen( &oid ) ;
      //genLobData( lobBuffer, lobSize ) ;
      bson obj ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "Write_OID", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      bson_destroy( &obj  ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // *(oids+i) = oid ;  -> stack smashing [stack buffer overflows]
      oids[i] = oid ;
      // get lob size and create time
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbGetLobSize( lob, &getSize ) ; // size
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbGetLobCreateTime( lob, &millis ) ; // create time
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // put in a normal record
      bson_init( &obj ) ;
      bson_append_oid( &obj, "lobOid", &oid ) ;
      bson_append_long( &obj, "lobSize", getSize ) ;
      bson_append_timestamp2( &obj, "lobCreateTime", millis/1000, millis %1000 ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      rc = sdbInsert( cl, &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      bson_destroy( &obj ) ;
      //memset( lobBuffer, 0, lobSize ) ;
   }
}


// read lob
void splitLobRead( sdbCollectionHandle cl, CHAR *lobBuffer,
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

   // thread wait
   //boost::this_thread::sleep( boost::posix_time::seconds( 1 ) ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      oid = *(oids+i) ;
      bson obj ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "OID", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      bson_destroy( &obj ) ;
      memset( lobBuffer, 0, lobSize ) ;

      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, rmd5, ENCRYTED_STR_LEN ) ;   // get md5 code read
      ASSERT_EQ( SDB_OK, rc ) ;
      ASSERT_STREQ( md5, rmd5 ) ;   // compare md5
      ASSERT_EQ( lobSize, readLen ) ;
   }
}

// remove lob
void splitLobRemove( sdbCollectionHandle cl, CHAR *lobBuffer,
                     const UINT64 lobSize, UINT64 putNum, bson_oid_t *oids )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   UINT32 i = 0 ;
   INT32 rc = SDB_OK ;
   UINT32 readLen = 0 ;
   bson_oid_t oid ;
   bson obj ;

   for( i = 0 ; i < putNum ; ++i )
   {
      oid = *(oids+i) ;
      bson_init( &obj ) ;
      //printf( "remove Lob begin\n" ) ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      //printf( "remove lob over\n" ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_FNE, rc ) ;
      // delete lob and then delete normal lobs
      bson_init( &obj ) ;
      bson_append_oid( &obj, "lobOid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      rc = sdbDelete( cl, &obj, NULL ) ;
      bson_print( &obj ) ;
      bson_destroy( &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
}
*/

// split
/*
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
*/
/*******************************************************************************
*@Description : create collection space specify lob page size, which located
*               in { 0, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288}
*               abnormal testcase in lobAbnormalTestcase.cpp
*@Modify List :
*               2014-8-22   xiaojun Hu   Init
*******************************************************************************/

/*
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
      rc = lobCreateCollectionPz( &db, &cs, prefName, &cl, clName, &lobPzObj ) ;
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
      rc = sdbDropCollection( cs, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
*/
