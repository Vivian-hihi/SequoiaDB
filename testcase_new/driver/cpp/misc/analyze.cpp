/**************************************************
* seqDB-11667:c++驱动支持统计
* test case for analyze
*
**************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <vector>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

const char* csname = "analyzeTestCs" ;
const char* clname = "analyzeTestCl" ;
sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;

// explain query { a:100 }
INT32 explainDoc( sdbCollection& cl, string& scanType )
{
	int rc = SDB_OK ;
	sdbCursor cursor ;
	BSONObj cond = BSON( "a" << 100 ) ;
	BSONObj obj ;
	rc = cl.explain( cursor, cond ) ;
	CHECK_RC( rc, "fail to explain" ) ;
	rc = cursor.next( obj ) ;
	CHECK_RC( rc, "fail to get next" ) ;
	scanType = obj.getField( "ScanType" ).String() ;
done:
	return rc ;
error:
	goto done ;
}

TEST( analyzeTest, explain )
{
	int rc = SDB_OK ;
	string scanType ;
	
	// connect create cs cl
	rc = createNormalCl( db, cs, cl, csname, clname ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// create index
	BSONObj indexDef = BSON( "a" << 1 ) ;
	rc = cl.createIndex( indexDef, "aIndex", false, false ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create index" ;

	// insert 10000 docs { a:100, b:98765 }
	vector<BSONObj> docs ;
	int i ;
	for( i = 0;i < 10000;i++ )
	{
		BSONObj doc = BSON( "a" << 100 << "b" << "98765" ) ;
		docs.push_back( doc ) ;
	}
	rc = cl.bulkInsert( 0, docs ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to bulk insert docs" ;

	// explain query { a:100 } before analyze
	rc = explainDoc( cl, scanType ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	ASSERT_STREQ( scanType.c_str(), "ixscan" ) ;

	// analyze
	BSONObj option = BSON( "CollectionSpace" << csname ) ;
	rc = db.analyze( option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to analyze" ;

	// explain query { a:100 } after analyze
	rc = explainDoc( cl, scanType ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	ASSERT_STREQ( scanType.c_str(), "tbscan" ) ;

	// drop cs
	rc = db.dropCollectionSpace( csname ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs" ;
}
