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
#include <pthread.h>
#include <unistd.h>
#include "testcommon.h"

#define ENCRYTED_STR_LEN   ( SDB_MD5_DIGEST_LENGTH * 2 + 1 )
/*******************************************************************************
*@Description : Test large object Open and Close LOB function.
*               { TestCase Number : LOB.FUNCTIONAL_TEST.FUNDAFUNC.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobOpenClose )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Just_Open_Close" ;
   CHAR clName[70] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;

   // Create collection for Open LOB
   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Generate OID and Open LOB
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Check Open LOB OVER>=*\n" ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Check Colse LOB OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc )  ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test large object Write LOB, Read LOB and Remove LOB function.
*               { TestCase Number : LOB.FUNCTIONAL_TEST.FUNDAFUNC.002 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobWriteReadRemove )
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
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid, rep_oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   if( NULL == (lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // Write Large Object
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$Oid", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   genLobData( lobBuffer, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = md5Code( lobBuffer, wMd5, ENCRYTED_STR_LEN ) ;  //Get MD5 from write
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Write LOB OVER>=*\n" ) ;
   // Get the Size of Lob and Check Write Lob Function
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read Large Object
   memset( lobBuffer, 0, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   printf( "*=<Read LOB OVER>=*\n" ) ;
   // Compare Read Size and Write Size
   ASSERT_TRUE( lobSize == sizeWrite ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Check Remove LOB function
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) ;
   printf( "*=<Remove LOB OVER>=*\n" ) ;
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
*@Description : Test large object seek read, {seekSize : [0,50,100]}.
*               { TestCase Number : LOB.FUNCTIONAL_TEST.FUNDAFUNC.005 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobSeekRead )
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
   CHAR clName[70] = { 0 } ;
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
   genLobData( lobBuffer, lobSize ) ;
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
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Seek Read Large Object
   UINT32 seekSize[] = { 0, 10000, 0, 0 } ;
   seekSize[2] = lobSize/2 ;
   seekSize[3] = sizeWrite - 1 ;
   UINT32 i ;
   for( i = 0 ; i < 4 ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize[i], SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      printf( "*=<Seek Read LOB OVER [seek size : %d]>=*\n", seekSize[i] ) ;
      // Check the write and read is correct or not
      SINT64 sizeGap = sizeWrite - seekSize[i] ;
      ASSERT_EQ( sizeGap, lobRead ) << "seek size read : " << lobRead
                                    << "seek size : " << seekSize[i]
                                    << "size write : " << sizeWrite ;
      printf( "*=<Correct LOB Seek Read [seek size : %d]>=*\n", seekSize[i] ) ;
   }
   printf( "*=<Correct LOB Write and LOB Seek Read>=*\n" ) ;
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
*@Description : Test LOB repeat basic operation. write lob in collection, read
*               lob, seek read lob and drop lob in once loop.
*               { TestCase Number : LOB.FUNCTIONALTEST.FUNDAFUNC.006 }
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobRepeatBasicOperation1 )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*2 ;
   //const UINT64 lobSize = 1024*6 ;
   const CHAR *prefixClName = "Lob_Repeat_Operat_Same_CL_Onec" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR *readBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   //UINT32 putNum = 100 ;
   UINT32 putNum = 10 ;
   bson_oid_t oid, rep_oid ;
   UINT32 i ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read the file to Buffer
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   for( i = 0 ; i < putNum ; ++i )
   {
      printf( "Begin Put the %d LOB\n", i ) ;
      // Write Large Object
      genLobData( lobBuffer, lobSize ) ;
      bson_oid_gen( &oid ) ;
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
      // Read Large Object
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Compare Read LOB and Write LOB
      ASSERT_EQ( lobRead, sizeWrite ) << "write Size : " << sizeWrite
                                      << "read Size : " << lobRead ;
      // Seek Read Large Object
      memset( lobBuffer, 0, lobSize ) ;
      UINT32 seekSize = lobSize/2 + 31 ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Repeated Write Seek Read data to new Large Object
      bson_oid_gen( &rep_oid ) ;
      rc = sdbOpenLob( cl, &rep_oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
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
      // in the end, inialize memory
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbRemoveLob( cl, &oid ) ;
      rc = sdbRemoveLob( cl, &rep_oid ) ;
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
*@Description : Test LOB repeat basic operation. write lob , read
*               lob, seek read lob and drop lob in loop .
*               { TestCase Number : LOB.FUNCTIONALTEST.FUNDAFUNC.006 }
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobRepeatBasicOperation2 )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   //const UINT64 lobSize = 1024*16 ;
   const CHAR *prefixClName = "Lob_Operate_Same_CL_Multi" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = { 0 };
   UINT32 lobRead = 0 ;
   //UINT32 putNum = 100 ;
   UINT32 putNum = 10 ;
   bson_oid_t oid, rep_oid ;
   bson_oid_t Oids[ putNum ] ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefixClName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   // Read the file to Buffer
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   // Repeated Write LOB
   UINT32 i, bufferSize = 0 ;
   for( i = 0 ; i < putNum ; ++i )
   {
      bson_oid_gen( &oid ) ;
      genLobData( lobBuffer, lobSize ) ;
      Oids[i] = oid ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      bufferSize = lobSize ;
   }
   printf( "*=<Write %d LOB OVER>=*\n", putNum ) ;
   // Repeated Get the Size of LOB
   SINT64 sizeWrite = 0 ;
   for( i = 0 ; i < putNum ; ++i )
   {
      rc = sdbOpenLob( cl, &(Oids[i]), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      ASSERT_EQ( sizeWrite, bufferSize ) << "Get Size is wrong"
                                         << "\nGet Lob Size = " << sizeWrite
                                         << "\nPut Lob Size = " << bufferSize ;
   }
   printf( "*=<Get Size in %d LOB OVER>=*\n", putNum ) ;
   // Repeated Read LOB
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &(Oids[i]), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Compare Read LOB and Write LOB
      ASSERT_TRUE( lobRead == sizeWrite ) << "Write Lob : " << sizeWrite
                                          << "Read Lob : " << lobRead ;
   }
   printf( "*=<Read %d LOB OVER>=*\n", putNum ) ;
   // Repeated Seek Read LOB
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobBuffer, 0, lobSize ) ;
      UINT32 seekSize = 256*1024-7889 ;
      rc = sdbOpenLob( cl, &(Oids[i]), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Check the write and repeated write new LOB is correct or not
      SINT64 sizeGap = sizeWrite - lobRead ;
      ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !\n"
                                     << "\nSize of Write = " << sizeWrite
                                     << "\nSize of Repeated Write = " << lobRead ;
   }
   printf( "*=<Seek Read %d LOB OVER>=*\n", putNum ) ;
   // Repeated Remove LOB
   for( i = 0 ; i < putNum ; ++i )
   {
      rc = sdbRemoveLob( cl, &(Oids[i])) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbOpenLob( cl, &(Oids[i]), SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_FNE, rc ) ;
   }
   printf( "*=<Remove %d LOB OVER>=*\n", putNum ) ;
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
*@Description : Test LOB repeat basic operation in different cl. write lob , read
*               lob, seek read lob and drop lob in loop .
*               { TestCase Number : LOB.FUNCTIONALTEST.FUNDAFUNC.006 }
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobRepeatBasicOperation3 )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*16 ;
   const CHAR *prefixClName = "Lob_Operate_Diff_CL" ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   CHAR clName[70] = {0} ;
   UINT32 lobRead = 0 ;
   UINT32 putNum = 10 ;
   CHAR clname[putNum] ;
   bson_oid_t oid, rep_oid ;
   bson obj ;
   INT32 i = 0 ;
   if( NULL == ( lobBuffer = (CHAR*)calloc( lobSize, sizeof(char) ) ) )
   {
      perror( "lobBuffer" ) ;
      ASSERT_TRUE( false ) ;
   }
   for( i = 0 ; i < putNum ; ++i )
   {
      printf( "Begin Put the %d Lob\n", i ) ;
      sprintf( clname, "%s%d", prefixClName, i ) ;
      rc = lobCreateCollection( &db, &cs, clname, &cl, clName, COMMCSNAME ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Write Large Object
      genLobData( lobBuffer, lobSize ) ;
      bson_oid_gen( &oid ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Get the Size of Lob
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc )  ;
      SINT64 sizeWrite = 0 ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Read Large Object
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Compare Read LOB and Write LOB
      ASSERT_EQ( lobRead, sizeWrite ) << "Read Lob size is not equal Write Lob "
                                      << "size\nRead Lob Size =  " << lobRead
                                      << "\nGet Lob Size = " << sizeWrite ;
      // Seek Read Large Object
      memset( lobBuffer, 0, lobSize ) ;
      UINT32 seekSize = 256*1024 - 7833 ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Repeated Write Seek Read data to new Large Object
      bson_oid_gen( &rep_oid ) ;
      rc = sdbOpenLob( cl, &rep_oid, SDB_LOB_CREATEONLY, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbWriteLob( lob, lobBuffer, lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
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
      ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !\n"
                                     << "\nSize of Write = " << sizeWrite
                                     << "\nSize of Repeated Write = "
                                     << repSizeWrite ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbRemoveLob( cl, &rep_oid ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      // Clear the table[collection] in the end
      rc = sdbDropCollection( cs, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      memset( lobBuffer, 0, lobSize ) ;
      sdbReleaseCollection( cl ) ;
      sdbReleaseCS( cs ) ;
      sdbReleaseConnection( db ) ;
   }
   free( lobBuffer ) ;
   lobBuffer = NULL ;
}

/*******************************************************************************
*@Description : Test write lob in sequoiadb, then seek read lob.
*               seek size range from 0 to buffer length.
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobSeekLoopRead )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 18*1024 ;
   const CHAR *prefName = "LOB_Seek_Loop_Read" ;
   CHAR clName[70] = { 0 } ;
   CHAR *lobBuffer = NULL ;
   bson_oid_t oid ;
   bson obj ;

   rc = lobCreateCollection( &db, &cs, prefName, &cl, clName, COMMCSNAME ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to Create Collection, rc = " << rc ;
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

   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   UINT32 readLen = 0 ;
   UINT32 seekOff = 0 ;
   do
   {
      memset( lobBuffer, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbSeekLob( lob, seekOff, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbReadLob( lob, lobSize, lobBuffer, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) ;
      ASSERT_EQ( getSize, readLen ) << "wrong seek read\nsize = " << getSize
                                    << "seek read size = " << readLen ;
      ++seekOff ;
      --getSize ;
   }while( 0 != getSize ) ;
   printf( "seek times = %d\n", seekOff ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   free( lobBuffer ) ;
   lobBuffer = NULL ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}
