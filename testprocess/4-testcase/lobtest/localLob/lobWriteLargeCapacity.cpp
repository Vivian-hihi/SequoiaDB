#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gtest/gtest.h"
#include "client.h"
#include "testcommon.h"
#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )

TEST( WriteFileTest, PutLargeCapacityFile )
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
   CHAR *readBuf = NULL ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   const UINT32 putNum = 1000 ;
   UINT64 sumSize = lobSize*putNum ;
   bson_oid_t oid ;
   bson obj ;

   // Create collection for Open LOB
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc =sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open Lob in write, rc = " << rc ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   UINT32 i, size = 0 ;
   UINT8 md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
   md5_state_t mst ;
   md5_init( &mst ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Write Lob, rc = " << rc ;
      md5_append( &mst, (const md5_byte_t *)lobWriteBuf, lobSize ) ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   md5_finish( &mst, md5digest ) ;
   CHAR *wmd5 = wMd5 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
   {
      snprintf( wmd5, 3, "%02x", md5digest[i] ) ;
      wmd5 += 2 ;
   }
   printf( "====<Write MD5 = %s>====\n", wMd5 ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close Lob in write, rc = " << rc ;
   // Get Size of Lob
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open Lob in get Size, rc = " << rc ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get Size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in get Size, rc = " << rc ;
   printf( "Get size = %d\n", getSize ) ;
   ASSERT_EQ( getSize, sumSize ) << "Write ERROR Data"
                                 << "\nGet Lob Size = " << getSize
                                 << "\nPut Lob Size = " << sumSize ;
   if( NULL == ( lobReadBuf = (CHAR *)malloc( lobSize ) ) )
   {
      perror( "malloc_lobReadBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open Lob in get Size, rc = " << rc ;
   UINT64 seekSize = 0 ;
   md5_init( &mst ) ;
   md5digest[ SDB_MD5_DIGEST_LENGTH ] = { 0 } ;
   for( i = 0 ; i < putNum ; ++i )
   {
      //rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      //ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      seekSize += lobRead ;
      md5_append( &mst, (const md5_byte_t *)lobReadBuf, lobRead ) ;
   }
   md5_finish( &mst, md5digest ) ;
   CHAR *rmd5 = rMd5 ;
   for( i = 0 ; i < SDB_MD5_DIGEST_LENGTH ; ++i )
   {
      snprintf( rmd5, 3, "%02x", md5digest[i] ) ;
      rmd5 += 2 ;
   }
   ASSERT_STREQ( wmd5, rmd5 ) << "Write Lob Data is not equal Read Lob Data"
                              << "Write MD5 = " << wmd5
                              << "Read MD5 = " << rmd5 ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in get Size, rc = " << rc ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

TEST( acceptanceTest, lobWriteRead )
{
/*
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
   CHAR cwMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR crMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *readBuf = NULL ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   UINT8 md5digest [ SDB_MD5_DIGEST_LENGTH ] = {0} ;
   bson_oid_t oid ;
   bson obj ;

   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Seek Read File
   md5_state_t pms ;
   UINT64 seekSize = 0 ;
   md5_init( &pms ) ;
   size_t rclen = 0 ;
   if( NULL == ( sFile = fopen( fileName, "r+" ) ) )
      ASSERT_TRUE( false ) << "Failed to open file = " << fileName ;
   if( NULL == (readBuf = (CHAR*)malloc( lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for read buffer" ;
   }
   do
   {
      //fseek( sFile, seekSize, SEEK_SET ) ;
      rclen = fread( readBuf, 1, strlen(lobWriteBuf)/4, sFile ) ;
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
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
   rc = md5Code( lobWriteBuf, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
   printf( "========================================\n" ) ;
   printf( "Write MD5 = %s\n", wMd5 ) ;
   printf( "========================================\n" ) ;
   rc = sdbWriteLob( lob, lobWriteBuf, strlen(lobWriteBuf) ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   if( NULL == (lobReadBuf = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Read Large Object
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
   rc = md5Code( lobWriteBuf, rMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from read
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get MD5 from read file" << rc ;
   rc = sdbReadLob( lob, strlen(lobWriteBuf), lobReadBuf, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
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
*/
}

TEST( md5Test, largeFileMd5Append )
{
/*
@Attention: Malloc Buffer must enougth. Otherwith, will Wrong
*/
   CHAR *buffer =
"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1]   = { 0 } ;
   CHAR sMd5[ENCRYTED_STR_LEN + 1]   = { 0 } ;
   UINT8 md5digest [ SDB_MD5_DIGEST_LENGTH ] = {0} ;
   UINT8 smd5digest [ SDB_MD5_DIGEST_LENGTH ] = {0} ;
   CHAR *code = wMd5 ;
   CHAR *Code = sMd5 ;
   CHAR *src = buffer ;
   CHAR *buf = NULL ;
   md5_state_t st ;
   md5_state_t pms ;
   md5_init( &pms ) ;
   UINT32 i ;
   UINT32 offset = 14 ;
   printf( "go\n" ) ;
   UINT32 len = strlen( buffer ) ;
   for( i = 0 ; i < 2 ; ++i )
   {
      if( NULL == (buf = (CHAR*)malloc( sizeof(CHAR*)*1024*1024)) )
      {
         ASSERT_TRUE( false ) << "Failed to allocate memory for read buffer" ;
      }
      memcpy( buf, buffer+(len/(2-i)-len/2), len/2 ) ;
/*
      if( 0 == i )
         buf = "abcdefghijklmnoaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" ;
      else
         buf = "aaaaaaaaaaaaaaaaaaaaaaaaiaaaaaaaaaaaaaaaapqrstuvwxyz" ;
*/
      printf( "BUFF = %s\n", buf ) ;
      md5_append( &pms, (const md5_byte_t *)buf, strlen(buf) ) ;
      //memset( buf, 0, strlen(buf) ) ;
      free( buf ) ;
      buf = NULL ;
   }
   md5_finish( &pms, smd5digest ) ;
   printf( "DIGEST1 = %s\n", smd5digest ) ;
   for ( i = 0 ; i < SDB_MD5_DIGEST_LENGTH; i++ )
   {
      snprintf( Code, 3, "%02x", smd5digest[i]) ;
      Code += 2 ;
   }
   printf( "==================================\n" ) ;
   printf( "Split MD5 = %s\n", sMd5) ;
   printf( "==================================\n" ) ;

   md5_init ( &st ) ;
   printf( "BUFFER = %s\n", src ) ;
   md5_append ( &st, (const md5_byte_t *) src, strlen(src) ) ;
   md5_finish ( &st, md5digest ) ;
   printf( "digest %s\n", md5digest ) ;
   for ( i = 0 ; i < SDB_MD5_DIGEST_LENGTH; i++ )
   {
      snprintf( code, 3, "%02x", md5digest[i]) ;
      code += 2 ;
   }
   printf( "==================================\n" ) ;
   printf( "Global MD5 = %s\n", wMd5 ) ;
   printf( "==================================\n" ) ;

}

