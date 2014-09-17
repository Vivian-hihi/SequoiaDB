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
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "testcommon.h"

#define TAG_START_CHAR '['
#define TAG_END_CHAR   ']'
#define COMMENT_CHAR   '#'

/* display syntax error */
void displaySyntax ( CHAR *pCommand )
{
   printf ( "Syntax: %s <hostname> <servicename> <username> <password>"
            OSS_NEWLINE, pCommand ) ;
}

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
*@Description : create collection for LOB
*@Parameter   : <DB>: connection handle
*               <CS>: collection space handle
*               <prefixName>: the collection's prefix name passed in
*               <cl>: collection handle
*               <clName>: full of collection name passed out
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
INT32 tCreateCollection( sdbConnectionHandle *DB, sdbCSHandle *CS,
                         const CHAR *prefixName, sdbCollectionHandle *cl,
                         CHAR *clName )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   CHAR clname[256] = { 0 } ;
   INT32 pageSize = 0 ;

   rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to connect to sdb, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbGetCollectionSpace( db, COMMCSNAME, &cs ) ;
   if( SDB_OK != rc && SDB_DMS_CS_NOTEXIST != rc )
   {
      printf( "Failed to get collection space, rc = %d\n", rc ) ;
      goto error ;
   }
   *DB = db ;
   if( SDB_DMS_CS_NOTEXIST == rc )
   {
      rc = sdbCreateCollectionSpace( db, COMMCSNAME, pageSize, &cs ) ;
      if( SDB_OK != rc )
      {
         printf( "Failed to create collection space, rc = %d\n", rc ) ;
         goto error ;
      }
   }
   getUniqueName( prefixName, clname ) ;     // Get unique collection name
   strcpy( clName, clname ) ;
   rc = sdbDropCollection( cs, clName ) ;
   if( SDB_OK != rc && SDB_DMS_NOTEXIST != rc )
   {
      printf( "Failed to drop collection, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbCreateCollection( cs, clName, cl ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to create collection, rc = %d\n", rc ) ;
      goto error ;
   }
   *CS = cs ;
done:
   return rc ;
error:
   goto done ;
}

/*******************************************************************************
*@Description : Thread for open large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void *tOpenLob( void *LOB )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   struct Lob *lobarg = (struct Lob*)LOB ;
   sdbCollectionHandle cl = lobarg->CL ;
   bson_oid_t oid = lobarg->OID ;
   bool sameLob = lobarg->inspect ;
   UINT32 rc = SDB_OK ;
   if( "" == sameLob )
      sameLob = false ;

   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   if( false == sameLob )
   {
      if( SDB_OK != rc )
      {
         printf( "Failed to open lob in different concurrent LOB, rc = %d\n",
                 rc ) ;
         goto error ;
      }
   }
   else
   {
      if( SDB_LOB_IS_UNDER_CRT != rc )
      {
         printf( "Cannot to open same lob, rc = %d\n", rc ) ;
         goto error ;
      }
   }
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : thread for close large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void *tCloseLob( void *LOB )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   struct Lob *lobarg = (struct Lob*)LOB ;
   bool sameLob = lobarg->inspect ;
   UINT32 rc = SDB_OK ;
   if( "" == sameLob )
      sameLob = false ;

   rc = sdbCloseLob( &lob ) ;
   if( false == sameLob )
   {
      if( SDB_OK != rc )
      {
         printf( "Failed to close lob in different concurrent LOB, rc = %d\n",
                 rc ) ;
         goto error ;
      }
   }
   else
   {
      if( SDB_LOB_IS_UNDER_CRT != rc )
      {
         printf( "Cannot to close same lob, rc = %d\n", rc ) ;
         goto error ;
      }
   }
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : thread for remove large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void *tRemoveLob( void *LOB )
{
   struct Lob *lobarg = (struct Lob*)LOB ;
   sdbCollectionHandle cl = lobarg->CL ;
   bson_oid_t oid = lobarg->OID ;
   bool sameLob = lobarg->inspect ;
   UINT32 rc = SDB_OK ;
   if( "" == sameLob )
      sameLob = false ;

   rc = sdbRemoveLob( cl, &oid ) ;
   if( false == sameLob )
   {
      if( SDB_OK != rc )
      {
         printf( "Failed to open lob in different concurrent LOB, rc = %d\n",
                 rc ) ;
         goto error ;
      }
   }
   else
   {
      // Remove Operation will not check Oid exist
      if( SDB_OK !=rc )
      {
         printf( "Cannot to open same lob, rc = %d\n", rc ) ;
         goto error ;
      }
   }
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : thread for write large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void *tWriteLob( void *LOB )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   struct Lob *lobarg = (struct Lob*)LOB ;
   sdbCollectionHandle cl = lobarg->CL ;
   bson_oid_t oid = lobarg->OID ;
   CHAR *lobWriteBuf = lobarg->Buffer ;
   INT32 rc = SDB_OK ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to open Lob in write thread, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbWriteLob( lob, lobWriteBuf, strlen(lobWriteBuf) ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to write Lob in write thread, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbCloseLob( &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to close Lob in write thread, rc = %d\n", rc ) ;
      goto error ;
   }
   bson obj ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : thread for read large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void *tReadLob( void *LOB )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   struct Lob *lobarg = (struct Lob*)LOB ;
   sdbCollectionHandle cl = lobarg->CL ;
   bson_oid_t oid = lobarg->OID ;
   CHAR *lobReadBuf = lobarg->Buffer ;
   const UINT64 lobSize = 1024*1024*16 ;
   INT32 rc = SDB_OK ;
   UINT32 lobRead = 0 ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Error, Cannot open Lob in read when Lob is not created,"
              "rc = %d\n", rc ) ;
      goto error ;
   }
   bson obj ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to write Lob in read thread, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbCloseLob( &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to close Lob in read thread, rc = %d\n", rc ) ;
      goto error ;
   }
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : thread for get large object size
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-7-15   xiaojun Hu   Init
*******************************************************************************/
void *tGetLobSize( void *LOB )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   struct Lob *lobarg = (struct Lob*)LOB ;
   sdbCollectionHandle cl = lobarg->CL ;
   bson_oid_t oid = lobarg->OID ;
   CHAR *lobWriteBuf = lobarg->Buffer ;
   INT32 rc = SDB_OK ;
   SINT64 sizeWrite = 0 ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to open Lob in get LOB size, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to write Lob, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbCloseLob( &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to close Lob in get LOB size, rc = %d\n", rc ) ;
      goto error ;
   }
   lobarg->size = sizeWrite ;
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : thread for seek read large object
*@Parameter   : <LOB>: the struct for thread passed arguement
*@Modify List :
*               2014-8-15   xiaojun Hu   Init
*******************************************************************************/
void *tSeekReadLob( void *LOB )
{
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   struct Lob *lobarg = (struct Lob*)LOB ;
   sdbCollectionHandle cl = lobarg->CL ;
   bson_oid_t oid = lobarg->OID ;
   CHAR *lobReadBuf = lobarg->Buffer ;
   const UINT64 lobSize = lobarg->size ;
   UINT32 seekSize = 1033 ;
   INT32 rc = SDB_OK ;
   UINT32 lobRead = 0 ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Error, Cannot open Lob when Lob is not created, rc = %d\n", rc ) ;
      goto error ;
   }
   bson obj ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to seek lob, rc = %d\n", rc ) ;
      goto error ;
   }
   rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to write Lob in read thread, rc = %d\n", rc ) ;
      goto error ;
   }
   printf( "THREAD SEEK READ = %d\n", strlen(lobReadBuf) ) ;
   rc = sdbCloseLob( &lob ) ;
   if( SDB_OK != rc )
   {
      printf( "Failed to close Lob in read thread, rc = %d\n", rc ) ;
      goto error ;
   }
error:
   goto done ;
done:
   return rc ;
}

/*******************************************************************************
*@Description : Auto generate LOB Data write in SDB.
*@Parameter   : <lobWriteBuf>: used to story data ;
*               <size>: the size of auto generate data ;
*@Modify List :
*               2014-8-15   xiaojun Hu   Init
*******************************************************************************/
void genLobData( CHAR *lobWriteBuf, int size )
{
   const CHAR *head = "B==head:" ;
   const CHAR *tail = ":tail==E" ;
   const CHAR buf[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
   CHAR *buffer = NULL ;
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
   UINT64 bodySize = size-(strlen(head)+strlen(tail)) ;
   //printf( "Begin == %s\n", lobWriteBuf ) ;
   memset( lobWriteBuf, 0, size ) ;
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
   sprintf( lobWriteBuf, "%s%s%s", head, lobWriteBuf, tail ) ;
   free( buffer ) ;
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
