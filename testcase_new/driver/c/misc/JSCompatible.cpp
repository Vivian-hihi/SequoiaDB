/**************************************************************
* @Description: test case of $numberLong JSCompatible 
*				TestLink 10968  
* @Modify     : Liang xuewang Init
*			 	2017-01-09
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/testcommon.hpp"

sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbCSHandle cs         = SDB_INVALID_HANDLE ;
sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
char CsName[100] ;
char ClName[100] ;

class NumberLongTest : public testing::Test
{
public :
	// run before every test
	void SetUp() ;
	// run after every test
	void TearDown() ;
} ; 

void NumberLongTest::SetUp()
{
	int rc = SDB_OK ;

    // connect to sdb   
    getConf() ;
    rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
    ASSERT_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ;

    // create cs cl
    const char* CsModName = "C_drivertest_syncCs" ;
    getUniqueName( CsModName, CsName ) ;
    rc = sdbCreateCollectionSpace( db, CsName, SDB_PAGESIZE_4K, &cs ) ;
    ASSERT_RC( rc, "fail to create cs %s, rc = %d\n", CsName, rc ) ;
    const char* ClModName = "C_drivertest_syncCl" ;
    getUniqueName( ClModName, ClName ) ;
    rc = sdbCreateCollection( cs, ClName, &cl ) ;
    ASSERT_RC( rc, "fail to create cl %s, rc = %d\n", ClName, rc ) ;	
}

void NumberLongTest::TearDown()
{
	int rc = SDB_OK ;
	
	// drop cs release handle
	rc = sdbDropCollectionSpace( db, CsName ) ;
	ASSERT_RC( rc, "fail to drop cs %s, rc = %d\n", CsName, rc ) ;
	sdbDisconnect( db ) ;
	sdbReleaseCollection( cl ) ;
	sdbReleaseCS( cs ) ;
	sdbReleaseConnection( db ) ;
}

TEST_F( NumberLongTest, JSfalse )
{
	int rc = SDB_OK ;

	// insert int/long/double max min 
	int a[] = { -2147483648, 0, 2147483647 } ;  // -2^31 0 2^31-1
	long b[] = { -9223372036854775808, -9007199254740992, -9007199254740991, 1, 
				  9007199254740991, 9007199254740992, 9223372036854775807 } ; // -2^63 -2^53 -2^53+1 1 2^53-1 2^53 2^63-1
	bson obj ;
	bson_init( &obj ) ;
	for( int i = 0;i < sizeof(a)/sizeof(a[0]);i++ )
	{
		char key[10] ;
		sprintf( key, "%s%d", "int", i ) ;
		bson_append_int( &obj, key, a[i] ) ;
	}
	for( int i = 0;i < sizeof(b)/sizeof(b[0]);i++ )
	{
		char key[10] ;
        sprintf( key, "%s%d", "long", i ) ;
		bson_append_long( &obj, key, b[i] ) ;
	}
	bson_finish( &obj ) ;
	rc = sdbInsert( cl, &obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to insert data" ;
	bson_destroy( &obj ) ;

	// query data
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
	bson selector ;
	bson_init( &selector ) ;
	bson_append_start_object( &selector, "_id" ) ;
	bson_append_int( &selector, "$include", 0 ) ;
	bson_append_finish_object( &selector ) ;
	bson_finish( &selector ) ;
	rc = sdbQuery( cl, NULL, &selector, NULL, NULL, 0, -1, &cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to query data" ;
	bson_init( &obj ) ;
	rc = sdbNext( cursor, &obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;

	const char* expect = "{ \"int0\": -2147483648, \"int1\": 0, \"int2\": 2147483647, \"long0\": -9223372036854775808, \"long1\": -9007199254740992, \"long2\": -9007199254740991, \"long3\": 1, \"long4\": 9007199254740991, \"long5\": 9007199254740992, \"long6\": 9223372036854775807 }" ; 
	char real[1024] ;
	bson_sprint( real, 1024, &obj ) ;
	ASSERT_STREQ( expect, real ) << "fail to check query data" ;
	bson_destroy( &obj ) ;
	
	sdbReleaseCursor( cursor ) ;
}

TEST_F( NumberLongTest, JStrue )
{
    int rc = SDB_OK ;
	bson_set_js_compatibility( true ) ;
	
    // insert int/long/double max min 
    int a[] = { -2147483648, 0, 2147483647 } ;  // -2^31 0 2^31-1
    long b[] = { -9223372036854775808, -9007199254740992, -9007199254740991, 1,
                  9007199254740991, 9007199254740992, 9223372036854775807 } ; // -2^63 -2^53 -2^53+1 1 2^53-1 2^53 2^63-1
    bson obj ;
    bson_init( &obj ) ;
    for( int i = 0;i < sizeof(a)/sizeof(a[0]);i++ )
    {
        char key[10] ;
        sprintf( key, "%s%d", "int", i ) ;
        bson_append_int( &obj, key, a[i] ) ;
    }
    for( int i = 0;i < sizeof(b)/sizeof(b[0]);i++ )
    {
        char key[10] ;
        sprintf( key, "%s%d", "long", i ) ;
        bson_append_long( &obj, key, b[i] ) ;
    }
	bson_finish( &obj ) ;
    rc = sdbInsert( cl, &obj ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to insert data" ;
    bson_destroy( &obj ) ;

    // query data
    sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
    bson selector ;
    bson_init( &selector ) ;
    bson_append_start_object( &selector, "_id" ) ;
	bson_append_int( &selector, "$include", 0 ) ;
    bson_append_finish_object( &selector ) ;
    bson_finish( &selector ) ;
    rc = sdbQuery( cl, NULL, &selector, NULL, NULL, 0, -1, &cursor ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to query data" ;
    bson_init( &obj ) ;
    rc = sdbNext( cursor, &obj ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;
	const char* expect = "{ \"int0\": -2147483648, \"int1\": 0, \"int2\": 2147483647, \"long0\": { \"$numberLong\": \"-9223372036854775808\" }, \"long1\": { \"$numberLong\": \"-9007199254740992\" }, \"long2\": -9007199254740991, \"long3\": 1, \"long4\": 9007199254740991, \"long5\": { \"$numberLong\": \"9007199254740992\" }, \"long6\": { \"$numberLong\": \"9223372036854775807\" } }" ;
    char real[1024] ;
    bson_sprint( real, 1024, &obj ) ;
    ASSERT_STREQ( expect, real ) << "fail to check query data" ;
    bson_destroy( &obj ) ;

    sdbReleaseCursor( cursor ) ;
}

