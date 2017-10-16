#include <stdio.h>
#include <gtest/gtest.h>
#include "arguments.hpp"
#include "./common/testcommon.hpp"
#include "client.h"

TEST(lob, lob_global_test)
{
   INT32 rc = SDB_OK ;
   // initialize the word environment
   rc = initEnv( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd() ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // initialize local variables
   sdbConnectionHandle db            = 0 ;
   sdbCollectionHandle cl            = 0 ;
   sdbCursorHandle cur               = 0 ;
   sdbLobHandle lob                  = 0 ;
   INT32 NUM                         = 10 ;
   SINT64 count                      = 0 ;
   bson_oid_t oid ;
   bson obj ;
   #define BUFSIZE1 (1024 * 1024 * 3)
   //#define BUFSIZE1 ( 1024 * 2 )
   #define BUFSIZE2 (1024 * 1024 * 2)
   SINT64 lobSize = -1 ;
   UINT64 createTime = -1 ;
   UINT64 modificationTime = -1 ;
   CHAR buf[BUFSIZE1] = { 0 } ;
   CHAR readBuf[BUFSIZE2] = { 0 } ;
   UINT32 readCount = 0 ;
   CHAR c = 'a' ;
   for ( INT32 i = 0; i < BUFSIZE1; i++ )
   {
      buf[i] = c ;
   }
   // connect to database
   rc = sdbConnect ( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd(), &db ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // get collection
   rc = getCollection ( db, COLLECTION_FULL_NAME, &cl ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // open lob 
   bson_oid_gen( &oid ) ; 
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // get lob size 
   rc = sdbGetLobSize( lob, &lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( 0, lobSize ) ;
   // get lob create time
   rc = sdbGetLobCreateTime( lob, &createTime ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbGetLobModificationTime( lob, &modificationTime ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( createTime, modificationTime ) ;
   // write lob 
   rc = sdbWriteLob( lob, buf, BUFSIZE1 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // get lob size 
   rc = sdbGetLobSize( lob, &lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( BUFSIZE1, lobSize ) ;
   // write lob
   rc = sdbWriteLob( lob, buf, BUFSIZE1 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // get lob size
   rc = sdbGetLobSize( lob, &lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( 2 * BUFSIZE1, lobSize ) ;
   // get lob create time
   rc = sdbGetLobCreateTime( lob, &createTime ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // close lob
   rc = sdbCloseLob ( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // open lob with the mode SDB_LOB_READ
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ; 
   ASSERT_EQ( SDB_OK, rc ) ;
   // read lob
   rc = sdbReadLob( lob, 1000, readBuf, &readCount ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_TRUE( readCount > 0 ) ;
   for ( INT32 i = 0; i < readCount; i++ )
   {
      ASSERT_EQ( c, readBuf[i] ) ;
   }
   // read lob
   rc = sdbReadLob( lob, BUFSIZE2, readBuf, &readCount ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_TRUE( readCount > 0 ) ;
   for ( INT32 i = 0; i < readCount; i++ )
   {
      ASSERT_EQ( c, readBuf[i] ) << "readCount is: " << readCount 
         << ", c is: " << c << ", i is: " 
         << i << ", readBuf[i] is: " << readBuf[i] ;
   }
   // close lob
   rc = sdbCloseLob ( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // remove lob
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
    
   // disconnect the connection
   sdbDisconnect ( db ) ;
   //release the local variables
   sdbReleaseCursor ( cur ) ;
   sdbReleaseCollection ( cl ) ;
   sdbReleaseConnection ( db ) ;
}

