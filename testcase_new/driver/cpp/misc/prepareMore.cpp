/**************************************************
* seqDB-2504:c++驱动flag增加QUERY_PREPARE_MORE
* test case for query flag: QUERY_PREPARE_MORE
*
**************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

const char* csname = "prepareMoreTestCs" ;
const char* clname = "prepareMoreTestCl" ;
sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;

TEST( prepareMoreTest, flag )
{
	int rc = SDB_OK ;
	
	// connect and create cs cl
	rc = createNormalCl( db, cs, cl, csname, clname ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cl " << clname ;

	// insert doc
	BSONObj doc = BSON( "a" << 1 ) ;
	rc = cl.insert( doc ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to insert doc to cl " << clname ;

	// query doc 
	sdbCursor cursor ;
	rc = cl.query( cursor, doc, _sdbStaticObject, _sdbStaticObject,
                   _sdbStaticObject, 0, -1, QUERY_PREPARE_MORE ) ; 
	ASSERT_EQ( rc, SDB_OK ) << "fail to query doc in cl " << clname ;
	BSONObj obj ;
	rc = cursor.next( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;
	int value = obj.getField( "a" ).Int() ;
	ASSERT_EQ( value, 1 ) << "fail to check query result" ;

	// drop cs
	rc = db.dropCollectionSpace( csname ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs " << csname ;
	db.disconnect() ;
}
