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
*               { TestCase Number : LOB.ABNORMAL_TEST.001 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobAbnormalNOTExistLob )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbLobHandle lob1 = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
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
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ; // oid have no value, can Open?
   ASSERT_EQ( SDB_FNE, rc ) << "Failed to open LOB in check "
                                   << "OpenLob, rc = " << rc ;
   printf( "*=<Check Open LOB OVER>=*\n" ) ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) << "Failed to get lob size, rc = " << rc ;
   printf( "*=<Check Get LOB Size OVER>=*\n" ) ;
   UINT64 millis = 0 ;
   rc = sdbGetLobCreateTime( lob, &millis ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) << "Failed to get Lob create time,"
                                   << " rc = " << rc ;
   printf( "*=<Check Get LOB Create Time OVER>=*\n" ) ;
   rc = sdbCloseLob( &lob ) ; // lob handle don't be created
   ASSERT_EQ( SDB_INVALIDARG, rc ) << "Failed to close LOB in check CloseLob, "
                                   << "rc = " << rc ;
   printf( "*=<Check Colse LOB OVER>=*\n" ) ;
   rc = sdbRemoveLob( lob, &oid ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) << "Failed to remove LOB, rc = " << rc ;
   printf( "*=<Check Remove Lob OVER>=*\n" ) ;
   // Clear the table[collection] in the end
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to drop collection in the end" << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test while Writing or Reading LOB, Remove LOB.
*               { TestCase Number : LOB.ABNORMAL_TEST.002 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobAbnormalRemoveWhileOperation )
{
   sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   sdbCSHandle cs = SDB_INVALID_HANDLE ;
   sdbCSHandle cl = SDB_INVALID_HANDLE ;
   sdbLobHandle lob = SDB_INVALID_HANDLE ;
   sdbLobHandle lob1 = SDB_INVALID_HANDLE ;
   INT32 rc = SDB_OK ;
   const CHAR *prefixClName = "Lob_Write_Then_Read" ;
   INT32 writeMode = SDB_LOB_CREATEONLY ;
   INT32 readMode = SDB_LOB_READ ;
   CHAR clName[70] = { 0 } ;
   bson_oid_t oid ;
   bson obj ;

}

/*******************************************************************************
*@Description : Test Remove LOB file and then do basic operation.
*               { TestCase Number : LOB.ABNORMAL_TEST.003 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobAbnormalRemoveLOBFile )
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
   const UINT32 putLob = 10 ;
   INT32 rmLob = 0 ;
   const CHAR *removeLobFile1 = "rm -rf /home/users/hxj/sequoiadb/database/"
                                "standalone/C_DriverTest_GTest_CS.1.lobd";
   const CHAR *removeLobFile2 = "rm -rf /home/users/hxj/sequoiadb/database/"
                                "standalone/C_DriverTest_GTest_CS.1.lobm";
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
   for( i = 0 ; i < putLob ; ++i )
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Write Lob, rc = " << rc ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close Lob in write, rc = " << rc ;
   //rmLob = system( removeLobFile1 ) ;   // Remove file type : ***.lobd
   //rmLob = system( removeLobFile2 ) ;   // Remove file type : ***.lobm
   // Get Size of Lob
   rc = sdbOpenLob( cl, &oid, readMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open Lob in get Size, rc = " << rc ;
   SINT64 getSize = 0 ;
   rc = sdbGetLobSize( lob, &getSize ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get Size of LOB, rc = " << rc ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Close LOB in get Size, rc = " << rc ;
   printf( "Get size = %d\n", getSize ) ;

}

/*******************************************************************************
*@Description : Test when disk have no space, write Lob/Read Lob.
*               { TestCase Number : LOB.ABNORMAL_TEST.004 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobAbnormalNoDiskSpace )
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
   const UINT32 putLob = 10 ;
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
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   UINT32 i, size = 0 ;
   do
   {
      memset( lobWriteBuf, 0, lobSize ) ;
      genLobData( lobWriteBuf, lobSize ) ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Write Lob, rc = " << rc ;
   }while( -11 == rc ) ;
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
   rc = sdbCloseLob( &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to close Lob in write, rc = " << rc ;
   rc = sdbRemoveLob( cl, &oid ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Remove LOB, rc = " << rc ;
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to Drop Collection, rc = " << rc ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

/*******************************************************************************
*@Description : Test when do Lob operation, network interrupt.
*               { TestCase Number : LOB.ABNORMAL_TEST.005 }
*@Modify List :
*               2014-8-20   xiaojun Hu   Init
*******************************************************************************/
TEST( fundamentalFuncTest, lobAbnormalNetworkInterruption )
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
   const UINT32 putLob = 20 ;
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
   if( NULL == ( lobWriteBuf = (CHAR*)malloc( lobSize ) ) )
   {
      ASSERT_TRUE( false ) << "Failed to allocate memory for lob write buffer" ;
   }
   rc =sdbOpenLob( cl, &oid, writeMode, &lob ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to open Lob in write, rc = " << rc ;
   UINT32 i, size = 0 ;
   // When Write Variaty Large Object in Sdb, network was interupted
   for( i = 0 ; i < putLob ; ++i )
   {
      genLobData( lobWriteBuf, lobSize ) ;
      rc = sdbWriteLob( lob, lobWriteBuf, lobSize ) ;
      ASSERT_EQ( SDB_OK, rc ) << "Failed to Write Lob, rc = " << rc ;
   }
   free( lobWriteBuf ) ;
   lobWriteBuf = NULL ;
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
}
