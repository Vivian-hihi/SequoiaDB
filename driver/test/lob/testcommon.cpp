/******************************************************************************
 *
 *@ Name : common.c
 *@ Description : Common functions for sample programs
 *                This file does NOT include main function
 *
 ******************************************************************************/
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <boost/thread.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "testcommon.h"

#define TAG_START_CHAR '['
#define TAG_END_CHAR   ']'
#define COMMENT_CHAR   '#'

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
boost::mutex io_mutex ;
/*******************************************************************************
*@Description : Give your module name add head[sdbtest_] and tail[pid].
*@Parameter   : <modName> : your module name[type:cons CHAR *]
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
void getUniqueName( const CHAR *modName, CHAR getName[] )
{
   INT32 rc = SDB_OK ;
   const CHAR *uniqName = "sdbtest_" ;
   pid_t pid ;
   pid = getpid() ;
   sprintf( getName, "%s%s_%d", uniqName, modName, (unsigned int)pid ) ;
}

/*******************************************************************************
*
*@Description : Get MD5 code for string
*@Parameter   : <src>: String to put and create MD5
*               <code>: md5, 32 byte
*               <size>: size of code md5
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
int md5Code( const char *src, char *code, int size )
{
   int rc                                  = 0 ;
   int i                                   = 0 ;
   unsigned char md5digest [ SDB_MD5_DIGEST_LENGTH ] = {0} ;
   md5_state_t st ;
   /*
   if ( size <= 2*SDB_MD5_DIGEST_LENGTH )
   {
      rc = SDB_INVALIDARG ;
      goto error ;
   }
   */
   md5_init ( &st ) ;
   md5_append ( &st, (const md5_byte_t *) src, strlen(src) ) ;
   md5_finish ( &st, md5digest ) ;
   for ( ; i < SDB_MD5_DIGEST_LENGTH; i++ )
   {
      snprintf( code, 3, "%02x", md5digest[i]) ;
      code += 2 ;
   }
done :
   return rc ;
error :
   goto done ;
}

/*******************************************************************************
*@Description : create normal collection for LOB
*@Parameter   : <DB>: connection handle
*               <CS>: collection space handle
*               <prefixName>: the collection's prefix name passed in
*               <cl>: collection handle
*               <clName>: full of collection name passed out
*@Modify List :
*               2014-8-25   xiaojun Hu  Init
*******************************************************************************/
INT32 lobCreateNmCollection( sdbConnectionHandle *DB, sdbCSHandle *CS,
                           const CHAR *prefixName, sdbCollectionHandle *cl,
                           CHAR *clName, CHAR *csName )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clname[256] = { 0 } ;
   CHAR full_clName[512] = { 0 } ;
   INT32 pageSize = 0 ;
   bson obj ;

   rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   CHECK_RC_CODE( rc, "failed to connect to sdb" )
   rc = sdbGetCollectionSpace( db, csName, &cs ) ;
   if( SDB_DMS_CS_NOTEXIST == rc )
   {
      rc = sdbCreateCollectionSpace( db, csName, pageSize, &cs ) ;
      CHECK_RC_CODE( rc, "failed to create collection space" )
   }
   getUniqueName( prefixName, clname ) ;     // Get unique collection name
   strcpy( clName, clname ) ;
   *DB = db ;
   // make options
/*
   bson_init( &obj ) ;
   bson_append_start_object( &obj, "ShardingKey" ) ;
   bson_append_int( &obj, "no", -1 ) ;
   bson_append_finish_object( &obj ) ;
   bson_append_string( &obj, "ShardingType", "hash" ) ;
   bson_append_int( &obj, "Partition", 1024 ) ;
   bson_append_int( &obj, "ReplSize", 0 ) ;
   bson_append_bool( &obj, "Compressed", true ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;
*/
   rc = sdbCreateCollection( cs, clName, cl ) ;
   bson_destroy( &obj ) ;
   if( SDB_DMS_EXIST == rc ) // if have collection, then get
   {
      sprintf( full_clName, "%s%s%s", csName, ".", clName ) ;
      rc = sdbGetCollection( db, full_clName, cl ) ;
   }
   CHECK_RC_CODE( rc, "failed to create collection" )
   *CS = cs ;
done:
   return rc ;
error:
   goto done ;
}

/*******************************************************************************
*@Description : create collection for LOB
*@Parameter   : <DB>: connection handle
*               <CS>: collection space handle
*               <prefixName>: the collection's prefix name passed in
*               <cl>: collection handle
*               <clName>: full of collection name passed out
*@Modify List :
*               2014-8-25   xiaojun Hu  Init
*******************************************************************************/
INT32 lobCreateCollection( sdbConnectionHandle *DB, sdbCSHandle *CS,
                           const CHAR *prefixName, sdbCollectionHandle *cl,
                           CHAR *clName, CHAR *csName )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clname[256] = { 0 } ;
   CHAR full_clName[512] = { 0 } ;
   INT32 pageSize = 0 ;
   bson obj ;

   rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   CHECK_RC_CODE( rc, "failed to connect to sdb" )
   rc = sdbGetCollectionSpace( db, csName, &cs ) ;
   if( SDB_DMS_CS_NOTEXIST == rc )
   {
      rc = sdbCreateCollectionSpace( db, csName, pageSize, &cs ) ;
      CHECK_RC_CODE( rc, "failed to create collection space" )
   }
   getUniqueName( prefixName, clname ) ;     // Get unique collection name
   strcpy( clName, clname ) ;
   *DB = db ;
   // make options
   bson_init( &obj ) ;
   bson_append_start_object( &obj, "ShardingKey" ) ;
   bson_append_int( &obj, "no", -1 ) ;
   bson_append_finish_object( &obj ) ;
   bson_append_string( &obj, "ShardingType", "hash" ) ;
   bson_append_int( &obj, "Partition", 1024 ) ;
   bson_append_int( &obj, "ReplSize", 0 ) ;
   bson_append_bool( &obj, "Compressed", true ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;
   rc = sdbCreateCollection1( cs, clName, &obj, cl ) ;
   bson_destroy( &obj ) ;
   if( SDB_DMS_EXIST == rc ) // if have collection, then get
   {
      sprintf( full_clName, "%s%s%s", csName, ".", clName ) ;
      rc = sdbGetCollection( db, full_clName, cl ) ;
   }
   CHECK_RC_CODE( rc, "failed to create collection" )
   *CS = cs ;
done:
   return rc ;
error:
   goto done ;
}

/*******************************************************************************
*@Description : create collection for LOB
*@Parameter   : <DB>: connection handle
*               <CS>: collection space handle
*               <prefixName>: the collection's prefix name passed in
*               <cl>: collection handle
*               <clName>: full of collection name passed out
*@Modify List :
*               2014-8-25   xiaojun Hu  Init
*******************************************************************************/
INT32 lobCreateCollection1( sdbConnectionHandle *DB, sdbCSHandle *CS,
                           const CHAR *prefixName, sdbCollectionHandle *cl,
                           CHAR *clName, CHAR *csName )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clname[256] = { 0 } ;
   CHAR full_clName[512] = { 0 } ;
   INT32 pageSize = 0 ;
   bson obj ;

   rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   CHECK_RC_CODE( rc, "failed to connect to sdb" )
   rc = sdbGetCollectionSpace( db, csName, &cs ) ;
   if( SDB_DMS_CS_NOTEXIST == rc )
   {
      rc = sdbCreateCollectionSpace( db, csName, pageSize, &cs ) ;
      CHECK_RC_CODE( rc, "failed to create collection space" )
   }
   getUniqueName( prefixName, clname ) ;     // Get unique collection name
   strcpy( clName, clname ) ;
   *DB = db ;
   // make options
   bson_init( &obj ) ;
   bson_append_start_object( &obj, "ShardingKey" ) ;
   bson_append_int( &obj, "no", 1 ) ;
   bson_append_finish_object( &obj ) ;
   bson_append_string( &obj, "ShardingType", "range" ) ;
   //bson_append_int( &obj, "Partition", 1024 ) ;
   bson_append_int( &obj, "ReplSize", 0 ) ;
   bson_append_bool( &obj, "Compressed", true ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;
   rc = sdbCreateCollection1( cs, clName, &obj, cl ) ;
   bson_destroy( &obj ) ;
   if( SDB_DMS_EXIST == rc ) // if have collection, then get
   {
      sprintf( full_clName, "%s%s%s", csName, ".", clName ) ;
      rc = sdbGetCollection( db, full_clName, cl ) ;
   }
   CHECK_RC_CODE( rc, "failed to create collection" )
   *CS = cs ;
done:
   return rc ;
error:
   goto done ;
}

/*******************************************************************************
*@Description : create collection for LOB, specify pagesize
*@Parameter   : <DB>: connection handle
*               <CS>: collection space handle
*               <prefixName>: the collection's prefix name passed in
*               <cl>: collection handle
*               <clName>: full of collection name passed out
*@Modify List :
*               2014-8-25   xiaojun Hu  Init
*******************************************************************************/
INT32 lobCreateCollectionPz( sdbConnectionHandle *DB, sdbCSHandle *CS,
                             const CHAR *prefixName, sdbCollectionHandle *cl,
                             CHAR *clName, bson *lobPzObj )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clname[256] = { 0 } ;
   CHAR full_clName[512] = { 0 } ;
   INT32 pageSize = 0 ;
   const CHAR *csName = "Specify_Lob_Pagesize_Create_CS" ;
   bson obj ;


   rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   CHECK_RC_CODE( rc, "failed to connect to sdb" )
   do
   {
      rc = sdbDropCollectionSpace( db, csName ) ;
   }while( 0 != rc && -34 != rc ) ;
   printf( "drop collection space success\n" ) ;
   //CHECK_RC_CODE( rc, "failed to drop collection space" ) ;
   rc = sdbCreateCollectionSpaceV2( db, csName, lobPzObj, &cs ) ;
   CHECK_RC_CODE( rc, "failed to create collection space" ) ;
   getUniqueName( prefixName, clname ) ;     // Get unique collection name
   strcpy( clName, clname ) ;
   *DB = db ;
   // make options
   bson_init( &obj ) ;
   bson_append_start_object( &obj, "ShardingKey" ) ;
   bson_append_int( &obj, "no", -1 ) ;
   bson_append_finish_object( &obj ) ;
   bson_append_string( &obj, "ShardingType", "hash" ) ;
   bson_append_int( &obj, "Partition", 1024 ) ;
   bson_append_int( &obj, "ReplSize", 0 ) ;
   bson_append_bool( &obj, "Compressed", true ) ;
   bson_append_finish_object( &obj ) ;
   //bson_print( &obj ) ;
   rc = sdbCreateCollection1( cs, clName, &obj, cl ) ;
   bson_destroy( &obj ) ;
   if( SDB_DMS_EXIST == rc ) // if have collection, then get
   {
      sprintf( full_clName, "%s%s%s", csName, ".", clName ) ;
      rc = sdbGetCollection( db, full_clName, cl ) ;
   }
   CHECK_RC_CODE( rc, "failed to create collection" )
   *CS = cs ;
done:
   return rc ;
error:
   goto done ;
}
/*******************************************************************************
*@Description : thread for write large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void thrdWriteLob( const CHAR *prefName, CHAR *lobBuffer, const UINT64 lobSize,
                   bson_oid_t *oids, sdbCSHandle *_cs, CHAR _clName[] )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clName[70] = { 0 } ;
   //CHAR *lobBuffer = NULL ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   bson_oid_t oid ;
   bson obj, Obj ;

   bson_init( &Obj ) ;
   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   *_cs = cs ;      // put cs handle out
   memcpy( _clName, clName, 70 ) ;     // put clName out
   // write lob
   bson_oid_gen( &oid ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // get lob size and creat time
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   *oids = oid ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "LobOid", &oid ) ;
   bson_append_long( &obj, "LobSize", getSize ) ;
   bson_append_timestamp2( &obj, "LobCreateTime", millis/1000, millis%1000 ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   rc = sdbInsert( cl, &obj ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   bson_destroy( &obj ) ;
   sdbReleaseCollection( cl ) ;
   //sdbReleaseCS( cs ) ;  release in main thread
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : thread for read large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void thrdReadLob( const CHAR *prefName, const UINT64 lobSize, bson_oid_t oid )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   //const CHAR *prefName = "Lob_Concurrent_Write_Same_CL" ;
   CHAR clName[70] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   UINT32 lobRead = 0 ;
   bson Obj ;

   bson_init( &Obj ) ;
   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = ( CHAR* )calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // read lob
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // simple inspect lob read
   ASSERT_EQ( lobSize, lobRead ) ;
/*
   bson obj ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   printf( "read lob successful, " ) ;
   bson_print( &obj ) ;
   bson_destroy( &obj  ) ;
*/
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : thread for seek read large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-8-15   xiaojun Hu   Init
*******************************************************************************/
void thrdSeekReadLob( const CHAR *prefName, const UINT64 lobSize,
                      bson_oid_t oid, SINT64 seekSz, SDB_LOB_SEEK whence )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clName[70] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   SINT64 getSize = 0 ;
   UINT64 millis = 0 ;
   UINT32 lobRead = 0 ;
   bson Obj ;

   bson_init( &Obj ) ;
   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == ( lobBuffer = ( CHAR* )calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // seek read lob
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbSeekLob( lob, seekSz, whence ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // inspect seek read
   SINT64 szGap = lobSize - seekSz ;
   ASSERT_EQ( szGap, lobRead ) ;
/*
   bson obj ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   printf( "seek read lob successful, " ) ;
   bson_print( &obj ) ;
   bson_destroy( &obj  ) ;
*/
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : thread for remove large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void thrdRemoveLob( const CHAR *prefName, const bson_oid_t oid )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clName[70] = { 0 } ;
   bson obj ;
   bson Obj ;

   bson_init( &Obj ) ;
   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // remove lob
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
/*
   bson_init( &obj ) ;
   bson_append_oid( &obj, "OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   printf( "remove lob successful, " ) ;
   bson_print( &obj ) ;
   bson_destroy( &obj ) ;
*/
   // release handle
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Auto generate LOB Data write in SDB.
*@Parameter   : <lobWriteBuf>: used to story data ;
*               <size>: the size of auto generate data ;
*@Modify List :
*               2014-8-15   xiaojun Hu   Init
*******************************************************************************/
void genLobData( CHAR *lobBuffer, UINT64 size )
{
   const CHAR *head = "B==head:" ;
   const CHAR *tail = ":tail==E" ;
   const CHAR buf[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
   CHAR *buffer = NULL ;
   CHAR *lobWriteBuf = NULL ;
   UINT32 allocSize = 64*1024*1024 ;
   UINT32 putSize = 0 ;
   UINT32 putNum = 0 ;
   UINT32 putTmp= 0 ;
   UINT32 rc = SDB_OK ;
   UINT64 i ;
   if( NULL == ( buffer = (CHAR *)malloc( allocSize ) ) )
   {
      perror( "buffer" ) ;
      exit(1) ;
   }
   if( NULL == ( lobWriteBuf = (CHAR *)malloc( size ) ) )
   {
      perror( "lobWriteBuf" ) ;
      exit(1) ;
   }
   if( size > (strlen(head)+strlen(tail)) )
   {
      UINT64 bodySize = size-(strlen(head)+strlen(tail)) ;
      //printf( "Begin == %s\n", lobWriteBuf ) ;
      memset( lobWriteBuf, 0, size ) ;
      memset( lobBuffer, 0, size ) ;
      for( i = 0 ; i <= bodySize/allocSize ; ++i )
      {
         memset( buffer, 0, allocSize ) ;
         if( i == bodySize/allocSize )
            putSize = bodySize%allocSize ;
         else
            putSize = allocSize ;
         rc = putData( putSize, buffer ) ;
         if( SDB_OK != rc )
            printf( "Failed to put data\n" ) ;
         sprintf( lobWriteBuf, "%s%s", lobWriteBuf, buffer ) ;
      }
      sprintf( lobBuffer, "%s%s%s", head, lobWriteBuf, tail ) ;
   }
   else
   {
      memset( lobWriteBuf, 0, strlen(head)+strlen(tail) ) ;
      sprintf( lobWriteBuf, "%s%s", head, tail ) ;
      memset( lobBuffer, 0, size ) ;
      strncpy( lobBuffer, lobWriteBuf, size ) ;
   }
   free( buffer ) ;
   buffer = NULL ;
   free( lobWriteBuf ) ;
   buffer = NULL ;
}
// Nest function for Create Data
static INT32 putData( UINT32 putSize, CHAR *buffer )
{
   CHAR buf[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
   CHAR dest[27] = {0} ;
   UINT32 putNum = 0 ;
   UINT32 putTmp = 0 ;
   UINT32 i = 1 ;
   CHAR *tmpBuf = NULL ;
   UINT32 allocSize = 64*1024*1024 ;
   if( NULL == ( tmpBuf = (CHAR *)malloc( allocSize ) ) )
   {
      perror( "tmpBuf" ) ;
      exit(1) ;
   }
   memset( tmpBuf, 0 , allocSize ) ;
   sprintf( tmpBuf, "%s", buf ) ;
   do
   {
      putNum = 26*( 1 << i ) ;
      if( putNum <= putSize )
      {
         sprintf( tmpBuf, "%s%s", tmpBuf, tmpBuf ) ;
      }
      else
      {
         if( 1 == i )
         {
            sprintf( tmpBuf, "%s", buf ) ;
            putTmp = 26 ;
         }
         putSize -= putTmp ;
         if( putSize > 26 )
         {
            sprintf( buffer, "%s%s", buffer, tmpBuf ) ;
            putSize = putData( putSize, buffer ) ;
         }
         else
         {
            sprintf( buffer, "%s%s", buffer, tmpBuf ) ;
            memcpy( dest, buf, putSize%26 ) ;
            strcat( buffer, dest ) ;
            putSize = 0 ;
         }
      }
      putTmp = putNum ;
      ++i ;
   }while( 0 != putSize ) ;
   free( tmpBuf ) ;
   tmpBuf = NULL ;
   return putSize ;
}

// write lob
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

   printf( "<<<begint to lob write>>>\n" ) ;
   for( i = 0 ; i < putNum ; ++i )
   {
      bson_oid_gen( &oid ) ;
/*
      bson obj ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "Write_OID", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      bson_destroy( &obj  ) ;
*/
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
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
      bson_append_oid( &obj, "wLobOid", &oid ) ;
      bson_append_long( &obj, "lobSize", getSize ) ;
      bson_append_timestamp2( &obj, "lobCreateTime", millis/1000, millis %1000 ) ;
      bson_append_finish_object( &obj ) ;
      //bson_print( &obj ) ;
      rc = sdbInsert( cl, &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      bson_destroy( &obj ) ;
      //memset( lobBuffer, 0, lobSize ) ;
   }
   printf( "<<<finish write>>>\n" ) ;
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
/*
      bson obj ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "rLobOid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      bson_print( &obj ) ;
      bson_destroy( &obj ) ;
*/
      memset( lobBuffer, 0, lobSize ) ;

      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = md5Code( lobBuffer, rmd5, ENCRYTED_STR_LEN ) ;   // get md5 code read
      ASSERT_EQ( SDB_OK, rc ) ;
      //ASSERT_STREQ( md5, rmd5 ) ;   // compare md5
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
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_FNE, rc ) ;
      // delete lob and then delete normal lobs
      bson_init( &obj ) ;
      bson_append_oid( &obj, "rmLobOid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      rc = sdbDelete( cl, &obj, NULL ) ;
      //bson_print( &obj ) ;
      bson_destroy( &obj ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
   }
}

// get Group Name that collectin located in
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
          0 != strcmp( rgName, srcGroup ) &&
          0 != strcmp( "SYSCoord", rgName ) )
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

// Get All Groups
/*
INT32 lobGetRgName( sdbConnectionHandle db, CHAR dstGroup[] )
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
   bson_init( &lrgObj ) ;
   while( SDB_OK == rc )
   {
      if( 0 == strcmp( "SYSCatalogGroup", rgName ) )
      {
         continue ;
      }
      rc = sdbNext( cursor, &lrgObj ) ;
      bson_iterator_init( &it, &lrgObj ) ;
      rgName = bson_iterator_string( &it ) ;
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
