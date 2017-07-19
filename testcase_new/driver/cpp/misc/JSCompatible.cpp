/**************************************************************
* @Description: test case of $numberLong JSCompatible 
*				TestLink 10969  
* @Modify     : Liang xuewang Init
*			 	2017-01-09
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <string>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;
const char* csModName = "C_drivertest_jsCs" ;
char csName[100] ;
const char* clName = "C_drivertest_jsCl" ;

int setUp()
{
	int rc = SDB_OK ;
    getUniqueName( csModName, csName ) ;
    rc = createNormalCl( db, cs, cl, csName, clName ) ;
    CHECK_RC( rc, "fail to create normal cl, rc = %d\n", rc ) ;
done:
	return rc ;
error:
	goto done ;
}

int tearDown()
{
	int rc = SDB_OK ;
	rc = db.dropCollectionSpace( csName ) ;
	CHECK_RC( rc, "fail to drop cs %s, rc = %d\n", csName, rc ) ;
	db.disconnect() ;
done:
	return rc ;
error:
	goto done ;
}

TEST( NumberLongTest, JSfalse )
{
	int rc = SDB_OK ;
	rc = setUp() ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// insert int/long/double max min 
	int  a[] = { -2147483648, 0, 2147483647 } ;  // -2^31 0 2^31-1
	long long b[] = { -9223372036854775808, -9007199254740992, -9007199254740991, 1, 
				      9007199254740991, 9007199254740992, 9223372036854775807 } ; // -2^63 -2^53 -2^53+1 1 2^53-1 2^53 2^63-1

	BSONObjBuilder builder ;
	char key[10] ;
	for( int i = 0;i < sizeof(a)/sizeof(a[0]);i++ )
	{
		sprintf( key, "%s%d", "int", i ) ;
		builder.append( key, a[i] ) ;
	}
	for( int i = 0;i < sizeof(b)/sizeof(b[0]);i++ )
	{
		sprintf( key, "%s%d", "long", i ) ;
        builder.append( key, b[i] ) ;
	}
	BSONObj doc = builder.obj() ;

	rc = cl.insert( doc ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to insert data" ;

	// query data
	sdbCursor cursor ;
	BSONObj selector = BSON( "_id" << BSON( "$include" << 0 ) ) ;
	rc = cl.query( cursor, _sdbStaticObject, selector ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to query data" ;
	BSONObj obj ;
	rc = cursor.next( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;

	const char* expect = "{ \"int0\": -2147483648, \"int1\": 0, \"int2\": 2147483647,"
						 " \"long0\": -9223372036854775808, \"long1\": -9007199254740992,"
						 " \"long2\": -9007199254740991, \"long3\": 1, \"long4\": 9007199254740991,"
						 " \"long5\": 9007199254740992, \"long6\": 9223372036854775807 }" ; 
	string str = obj.toString() ;
	const char* real = str.c_str() ;
	ASSERT_STREQ( expect, real ) << "fail to check query data" ;

	rc = tearDown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}

TEST( NumberLongTest, JStrue )
{
    int rc = SDB_OK ;
	rc = setUp() ;
	ASSERT_EQ( rc, SDB_OK ) ;

	BSONObj::setJSCompatibility( true ) ;
	
    // insert int/long/double max min 
    int  a[] = { -2147483648, 0, 2147483647 } ;  // -2^31 0 2^31-1
    long long b[] = { -9223372036854775808, -9007199254740992, -9007199254740991, 1,
                  9007199254740991, 9007199254740992, 9223372036854775807 } ; // -2^63 -2^53 -2^53+1 1 2^53-1 2^53 2^63-1
    BSONObjBuilder builder ;
	char key[10] ;
    for( int i = 0;i < sizeof(a)/sizeof(a[0]);i++ )
    {
		sprintf( key, "%s%d", "int", i ) ;
        builder.append( key, a[i] ) ;
    }
    for( int i = 0;i < sizeof(b)/sizeof(b[0]);i++ )
    {
        sprintf( key, "%s%d", "long", i ) ;
		builder.append( key, b[i] ) ;
    }
	BSONObj doc = builder.obj() ;
    rc = cl.insert( doc ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to insert data" ;

    // query data
    sdbCursor cursor ;
    BSONObj selector = BSON( "_id" << BSON( "$include" << 0 ) ) ;
    rc = cl.query( cursor, _sdbStaticObject, selector ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to query data" ;
	BSONObj obj ;
    rc = cursor.next( obj ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;
	const char* expect = "{ \"int0\": -2147483648, \"int1\": 0, \"int2\": 2147483647,"
						 " \"long0\": { \"$numberLong\": \"-9223372036854775808\" },"
						 " \"long1\": { \"$numberLong\": \"-9007199254740992\" },"
						 " \"long2\": -9007199254740991, \"long3\": 1, \"long4\": 9007199254740991,"
						 " \"long5\": { \"$numberLong\": \"9007199254740992\" },"
						 " \"long6\": { \"$numberLong\": \"9223372036854775807\" } }" ;
    string str = obj.toString() ;
	const char* real = str.c_str() ;
    ASSERT_STREQ( expect, real ) << "fail to check query data" ;

	rc = tearDown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}
