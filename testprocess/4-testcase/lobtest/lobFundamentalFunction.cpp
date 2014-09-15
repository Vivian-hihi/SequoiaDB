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
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   CHAR clName[70] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;
   // Create collection for Open LOB
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Generate OID and Open LOB
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;  // Print the oid
   rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in check OpenLob, rc = " << rc ;
   printf( "*=<Check Open LOB OVER>=*\n" ) ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in check CloseLob, "
                               << "rc = " << rc ;
   printf( "*=<Check Colse LOB OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
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
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR clName[70] = { 0 } ;
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
   // Get the Size of Lob and Check Write Lob Function
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
   rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in read, rc = " << rc ;
   printf( "*=<Read LOB OVER>=*\n" ) ;
   // Compare Read Size and Write Size
   ASSERT_TRUE( lobSize == sizeWrite ) << "Error, wrong write "
                                                  << "and read function "
                                                  << ", rc = " << rc ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to remove LOB, rc = " << rc ;
   // Check Remove LOB function
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_FNE, rc ) << "Wrong operation in open LOB, rc =  " << rc ;
   printf( "*=<Remove LOB OVER>=*\n" ) ;
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
*@Description : Test large object seek read, {seekSize : [0,50,100]}.
*               { TestCase Number : LOB.FUNCTIONAL_TEST.FUNDAFUNC.005 }
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
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   bson_oid_t oid ;
   bson obj ;

   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Write Large Object
   genLobData( lobWriteBuf, lobSize ) ;
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
   // Get the Size of Lob
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
   SINT64 sizeWrite = 0 ;
   rc = sdbGetLobSize( lob, &sizeWrite ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
   // Seek Read Large Object
   UINT32 seekSize[] = { 0, 10000, 0, 0 } ;
   seekSize[2] = lobSize/2 ;
   seekSize[3] = sizeWrite - 1 ;
   UINT32 i ;
   if( NULL == (lobReadBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   for( i = 0 ; i < 4 ; ++i )
   {
      memset( lobReadBuf, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
      rc = sdbSeekLob( lob, seekSize[i], SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
      printf( "*=<Seek Read LOB OVER [seek size : %d]>=*\n", seekSize[i] ) ;
      // Check the write and read is correct or not
      SINT64 sizeGap = sizeWrite - seekSize[i] ;
      ASSERT_EQ( sizeGap, lobRead ) << "Failed to seek read !\n"
                                               << "\nSize of Write = " << sizeWrite
                                               << "\nSize of Seek Read = "
                                               << strlen(lobReadBuf) ;
      printf( "*=<Correct LOB Seek Read [seek size : %d]>=*\n", seekSize[i] ) ;
   }
   printf( "*=<Correct LOB Write and LOB Seek Read>=*\n" ) ;
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
*@Description : Test large object repeat basic operation. Same CL.
*               { TestCase Number : LOB.FUNCTIONALTEST.FUNDAFUNC.006 }
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobRepeatBasicOperation1 )
{
/*
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 1024*1024*60 ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR wMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR rMd5[ENCRYTED_STR_LEN + 1] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR *readBuf = NULL ;
   CHAR clName[70] = { 0 } ;
   UINT32 lobRead = 0 ;
   UINT32 putNum = 100 ;
   bson_oid_t oid, rep_oid ;
   bson obj ;
   UINT32 i ;
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Read the file to Buffer
   if( NULL == ( readBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   if( NULL == (lobReadBuf = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      strcpy( lobWriteBuf, readBuf ) ;
      // Write Large Object
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || putNum-1 == i )
         bson_print( &obj ) ;  // Print the oid
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
      // Read Large Object
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in read, rc = " << rc ;
      // Compare Read LOB and Write LOB
      ASSERT_EQ( lobRead, sizeWrite ) << "Read Lob size is not "
                                      << "equal Write Lob size"
                                      << ", rc = " << rc;
      memset( lobReadBuf, 0, lobSize ) ;
      // Seek Read Large Object
      UINT32 seekSize = 3*256*1024+4783 ;
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in seek read, rc = " << rc ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in seek read, rc = " << rc ;
      // Repeated Use [lobWriteBuf]
      memset( lobWriteBuf, 0, lobSize ) ;
      strncpy( lobWriteBuf, lobReadBuf, lobRead ) ;
      // Repeated Write Seek Read data to new Large Object
      bson_oid_gen( &rep_oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Rep_Oid", &rep_oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || putNum-1 == i )
         bson_print( &obj ) ;  // Print the oid
      rc = sdbOpenLob( cl, &rep_oid, writeMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
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
      rc = sdbRemoveLob( cl, &oid ) ;
      rc = sdbRemoveLob( cl, &rep_oid ) ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
*/
}

/*******************************************************************************
*@Description : Test large object repetitive basic operation. Same CL.
*               { TestCase Number : LOB.FUNCTIONALTEST.FUNDAFUNC.007}
*@Modify List :
*               2014-8-26   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobRepeatBasicOperation2 )
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
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   CHAR *readBuf = NULL ;
   CHAR clName[70] = { 0 };
   UINT32 lobRead = 0 ;
   UINT32 putNum = 100 ;
   bson_oid_t oid, rep_oid ;
   bson_oid_t Oids[ putNum ] ;
   bson obj ;
   rc = tCreateCollection( &db, &cs, prefixClName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
   // Read the file to Buffer
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   // Repeated Write LOB
   UINT32 i, bufferSize = 0 ;
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Oid", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || putNum-1 == i )
         bson_print( &obj ) ;  // Print the oid
      Oids[i] = oid ;
      rc = sdbOpenLob( cl, &oid, writeMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
      bufferSize = lobSize ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   printf( "*=<Write %d LOB OVER>=*\n", putNum ) ;
   // Repeated Get the Size of LOB
   SINT64 sizeWrite = 0 ;
   for( i = 0 ; i < putNum ; ++i )
   {
      rc = sdbOpenLob( cl, &(Oids[i]), readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in get size, rc = " << rc ;
      rc = sdbGetLobSize( lob, &sizeWrite ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to get size of LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in get size, rc = " << rc ;
      ASSERT_EQ( sizeWrite, bufferSize ) << "Get Size is wrong"
                                         << "\nGet Lob Size = " << sizeWrite
                                         << "\nPut Lob Size = " << bufferSize ;
   }
   printf( "*=<Get Size in %d LOB OVER>=*\n", putNum ) ;
   if( NULL == ( lobReadBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   // Repeated Read LOB
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobReadBuf, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &(Oids[i]), readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
      rc = sdbReadLob( lob, bufferSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in read, rc = " << rc ;
      // Compare Read LOB and Write LOB
      ASSERT_TRUE( lobRead == sizeWrite ) << "Read Lob size is not "
                                                       << "equal Write Lob size"
                                                       << ", rc = " << rc;
   }
   printf( "*=<Read %d LOB OVER>=*\n", putNum ) ;
   // Repeated Seek Read LOB
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobReadBuf, 0, lobSize ) ;
      UINT32 seekSize = 256*1024-7889 ;
      rc = sdbOpenLob( cl, &(Oids[i]), readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in seek read, rc = " << rc ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
      rc = sdbReadLob( lob, bufferSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in seek read, rc = " << rc ;
      // Check the write and repeated write new LOB is correct or not
      SINT64 sizeGap = sizeWrite - lobRead ;
      ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !\n"
                                     << "\nSize of Write = " << sizeWrite
                                     << "\nSize of Repeated Write = " << lobRead ;
   }
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
   printf( "*=<Seek Read %d LOB OVER>=*\n", putNum ) ;
   // Repeated Remove LOB
   for( i = 0 ; i < putNum ; ++i )
   {
      rc = sdbRemoveLob( cl, &(Oids[i])) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to remove LOB, rc = " << rc ;
      rc = sdbOpenLob( cl, &(Oids[i]), readMode, &lob ) ;
      ASSERT_TRUE( SDB_FNE == rc ) << "Remove Lob operation  Wrong, rc = " << rc ;
   }
   printf( "*=<Remove %d LOB OVER>=*\n", putNum ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
*/
}

/*******************************************************************************
*@Description : Test large object repetitive basic operation. Same CL.
*               { TestCase Number : LOB.FUNCTIONALTEST.FUNDAFUNC.008 }
*@Modify List :
*               2014-8-25   xiaojun Hu   Init
*******************************************************************************/
TEST( acceptanceTest, lobRepeatBasicOperation3 )
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
   CHAR clName[70] = {0} ;
   UINT32 lobRead = 0 ;
   UINT32 putNum = 100 ;
   CHAR clname[putNum] ;
   bson_oid_t oid, rep_oid ;
   bson obj ;
   INT32 i = 0 ;
   if( NULL == (lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   if( NULL == (lobReadBuf = (CHAR*)malloc( sizeof(CHAR*)*lobSize )) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob read buffer" ;
   }
   for( i = 0 ; i < putNum ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      memset( lobReadBuf, 0, lobSize ) ;
      sprintf( clname, "%s%d", prefixClName, i ) ;
      rc = tCreateCollection( &db, &cs, clname, &cl, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to run create CL, rc = " << rc  ;
      //if( 0 == i )
      //   printf( "{\"Collection Name\" : \"%s\"}\n", clName ) ;
      // Write Large Object
      bson_oid_gen( &oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$OID", &oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || putNum-1 == i )
         bson_print( &obj ) ;  // Print the oid
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
      // Read Large Object
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in read, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in read, rc = " << rc ;
      // Compare Read LOB and Write LOB
      ASSERT_EQ( lobRead, sizeWrite ) << "Read Lob size is not equal Write Lob "
                                      << "size\nRead Lob Size =  " << lobRead
                                      << "\nGet Lob Size = " << sizeWrite ;
      memset( lobReadBuf, 0, lobSize ) ;
      // Seek Read Large Object
      UINT32 seekSize = 256*1024 - 7833 ;
      rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in seek read, rc = " << rc ;
      rc = sdbSeekLob( lob, seekSize, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to seek lob, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to read LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in seek read, rc = " << rc ;
      // Repeated Use [lobWriteBuf]
      memset( lobWriteBuf, 0, lobSize ) ;
      strncpy( lobWriteBuf, lobReadBuf, lobRead ) ;
      // Repeated Write Seek Read data to new Large Object
      bson_oid_gen( &rep_oid ) ;
      bson_init( &obj ) ;
      bson_append_oid( &obj, "$Rep_Oid", &rep_oid ) ;
      bson_append_finish_object( &obj ) ;
      if( 0 == i || putNum-1 == i )
         bson_print( &obj ) ;  // Print the oid
      rc = sdbOpenLob( cl, &rep_oid, writeMode, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to open LOB in write, rc = " << rc ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobRead ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to write LOB, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to close LOB in write, rc = " << rc ;
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
      ASSERT_EQ( sizeGap, seekSize ) << "Failed to seek read !\n"
                                     << "\nSize of Write = " << sizeWrite
                                     << "\nSize of Repeated Write = "
                                     << repSizeWrite ;
      rc = sdbRemoveLob( cl, &oid ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove Lob, rc = " << rc ;
      rc = sdbRemoveLob( cl, &rep_oid ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
      // Clear the table[collection] in the end
      rc = sdbDropCollection( cs, clName ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
      sdbReleaseCollection( cl ) ;
      sdbReleaseCS( cs ) ;
      sdbReleaseConnection( db ) ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
}

TEST( fundamentalFuncTest, lobSeekLoopRead )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const UINT64 lobSize = 18*1024*1024 ;
   const CHAR *prefName = "LOB_Seek_Loop_Read" ;
   CHAR clName[70] = { 0 } ;
   CHAR *lobWriteBuf = NULL ;
   CHAR *lobReadBuf = NULL ;
   bson_oid_t oid ;
   bson obj ;
   rc = tCreateCollection( &db, &cs, prefName, &cl, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to Create Collection, rc = " << rc ;
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      perror( "malloc_lobWriteBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   if( NULL == ( lobReadBuf = (CHAR*)malloc( lobSize ) ) )
   {
      perror( "malloc_lobReadBuf" ) ;
      ASSERT_TRUE( false ) ;
   }
   bson_oid_gen( &oid ) ;
   bson_init( &obj ) ;
   bson_append_oid( &obj, "$OID", &oid ) ;
   bson_append_finish_object( &obj ) ;
   bson_print( &obj ) ;
   memset( lobWriteBuf, 0, lobSize ) ;
   genLobData( lobWriteBuf, lobSize ) ;
   rc = sdbOpenLob( cl, &oid, SDB_LOB_CREATEONLY, &lob ) ;
   ASSERT_EQ( SDB_OK, rc )<< "failed to open lob in write, rc = " << rc ;
   rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to write lob, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to close lob in write, rc = " << rc ;

   rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to open lob in read size, rc = " << rc ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to get lob size, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to close lob in read size, rc = " << rc ;

   UINT32 readLen = 0 ;
   UINT32 seekOff = 0 ;
   do
   {
      memset( lobReadBuf, 0, lobSize ) ;
      rc = sdbOpenLob( cl, &oid, SDB_LOB_READ, &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "failed to open lob in seek read, rc = " << rc ;
      rc = sdbSeekLob( lob, seekOff, SDB_LOB_SEEK_SET ) ;
      ASSERT_EQ( SDB_OK, rc ) << "failed to seek lob, rc = " << rc ;
      rc = sdbReadLob( lob, lobSize, lobReadBuf, &readLen ) ;
      ASSERT_EQ( SDB_OK, rc ) << "failed to read lob, rc = " << rc ;
      rc = sdbCloseLob( &lob ) ;
      ASSERT_EQ( SDB_OK, rc ) << "failed to close lob in seek read, rc = " << rc ;
      ASSERT_EQ( getSize, readLen ) << "wrong seek read\nsize = " << getSize
                                    << "seek read size = " << readLen ;
      ++seekOff ;
      --getSize ;
   }while( 0 == getSize ) ;
   printf( "seek times = %s\n", seekOff ) ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to remove lob, rc = " << rc ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "failed to drop collection, rc = " << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   free( lobReadBuf ) ;
   lobReadBuf = NULL ;
}
