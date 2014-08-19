#include "client.h"

#include <gtest/gtest.h>

using namespace std ;



TEST(lobTest, insert_1)
{
   INT32 rc = SDB_OK ;
   sdbConnectionHandle conn = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   bson_oid_t oid ;

   rc = sdbConnect( "localhost", "11810", "", "", &conn ) ;
   ASSERT_TRUE( SDB_OK == rc ) ;

   rc = sdbGetCollection( conn, "foo.bar", &cl ) ;
   ASSERT_TRUE( SDB_OK == rc ) ;

   const UINT32 putNum = 100 ;
   bson_oid_t oids[putNum] ;
   const UINT32 bufSize = 1231 ;
   CHAR buf[bufSize] = { 0 } ;

   for ( UINT32 i = 0 ; i < putNum ; ++i )
   {
      bson_oid_gen( &oid ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      oids[i] = oid ;
      memset( buf, 'a' + i, bufSize ) ;
      rc = sdbWriteLob( lob, buf, bufSize ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
   }

   cout << "write done" << endl ;

   CHAR buf2[bufSize] = { 0 } ;
   for ( UINT32 i = 0 ; i < putNum ; ++i )
   {
      rc = sdbOpenLob( cl, &( oids[i] ), SDB_LOB_READ, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      SINT64 lobSize = 0 ;
      rc = sdbGetLobSize( lob, &lobSize ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      ASSERT_TRUE( bufSize == lobSize ) ;
      UINT32 readSize = 0 ;
      rc = sdbReadLob( lob, bufSize, buf, &readSize ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      ASSERT_TRUE( readSize = bufSize ) ;
      memset( buf2, 'a' + i, bufSize ) ;
      ASSERT_TRUE( 0 == memcmp(buf, buf2, bufSize )) ;
      rc = sdbReadLob( lob, bufSize, buf, &readSize ) ;
      ASSERT_TRUE( SDB_EOF == rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      rc = sdbRemoveLob( cl, &( oids[i] ) ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
   }
}

TEST(lobTest, insert_2)
{
   INT32 rc = SDB_OK ;
   sdbConnectionHandle conn = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   bson_oid_t oid ;

   rc = sdbConnect( "localhost", "11810", "", "", &conn ) ;
   ASSERT_TRUE( SDB_OK == rc ) ;

   rc = sdbGetCollection( conn, "foo.bar", &cl ) ;
   ASSERT_TRUE( SDB_OK == rc ) ;

   const UINT32 putNum = 1000 ;
   const UINT32 bufSize = 1024 * 1024 * 10 + 1231 ;
   CHAR *buf = new CHAR[bufSize] ;
   CHAR *buf2 = new CHAR[bufSize] ;
   bson_oid_t oids[putNum] ;

   for ( UINT32 i = 0 ; i < putNum ; ++i )
   {
      for ( UINT32 j = 0 ; j < bufSize; ++j )
      {
         buf[j] = ( CHAR )rand() ;
      }

      bson_oid_gen( &oid ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      oids[i] = oid ;
      memset( buf, 'a' + i, bufSize ) ;
      SINT64 totalWriteLen = 0 ;
      while ( totalWriteLen < bufSize )
      {
         UINT32 writeLen = bufSize - totalWriteLen < 1452 ? bufSize - totalWriteLen : 1452 ;
         rc = sdbWriteLob( lob, buf + totalWriteLen, writeLen ) ;
         ASSERT_TRUE( SDB_OK == rc ) ;
         totalWriteLen += writeLen ;
      }
      ASSERT_TRUE( totalWriteLen == bufSize ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;

      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      SINT64 lobSize = 0 ;
      rc = sdbGetLobSize( lob, &lobSize ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      ASSERT_TRUE( bufSize == lobSize ) ;
      SINT64 totalReadLen = 0 ;
      UINT32 readLen = 0 ;
      const UINT32 readSize = 456 ;
      while ( totalReadLen < lobSize )
      { 
         readLen = 0 ;
         rc = sdbReadLob( lob, readSize, buf2 + totalReadLen, &readLen ) ;
         ASSERT_TRUE( SDB_OK == rc ) ;
         totalReadLen += readLen ;
      }
      ASSERT_TRUE( 0 == memcmp(buf, buf2, bufSize )) ;
      rc = sdbReadLob( lob, readSize, buf2, &readLen ) ;
      ASSERT_TRUE( SDB_EOF == rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
      CHAR oidstr[25] ;
      bson_oid_to_string( &oid, oidstr ) ;
      cout << "oid done:" << oidstr << endl ;
   }

   for ( UINT32 i = 0 ; i < putNum ; ++i )
   {
      rc = sdbRemoveLob( cl, &(oids[i])) ;
      ASSERT_TRUE( SDB_OK == rc ) ;
   }

   delete []buf ;
   delete []buf2 ;
}

