/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1110
*               SEQUOIADBMAINSTREAM-849
* @Modify     : Liang xuewang Init
*			 	2017-07-20
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

char *CsModName = "cpp_driver_test" ;
char CsName[100] ;
char *ClName = "idIndex" ;
sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;

int setup()
{
	int rc = SDB_OK ;
	BSONObj option ;
	int num = 2000, i ;
    vector<BSONObj> docs ;

	getConf() ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	CHECK_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ;

	getUniqueName( CsModName, CsName ) ;
	rc = db.createCollectionSpace( CsName, SDB_PAGESIZE_4K, cs ) ;
	CHECK_RC( rc, "fail to create cs %s, rc = %d\n", CsName, rc ) ;

	// option is { ReplSize: 0 }, { Compressed: true }, { AutoIndexId: false }
	option = BSON( "ReplSize" << 0 << "Compressed" << true << "AutoIndexId" << false ) ;

	// create cl
	rc = cs.createCollection( ClName, option, cl ) ;
	CHECK_RC( rc, "fail to create cl %s, rc = %d\n", ClName, rc ) ;

	// insert records like { "_id": 1, "f1": 2, "f2": 3 }
	for( i = 0;i < num;++i )
	{
		BSONObj doc = BSON( "_id" << i << "f1" << (i+1) << "f2" << (i+2) ) ;
		docs.push_back( doc ) ;
	}
	rc = cl.bulkInsert( 0, docs ) ;
	CHECK_RC( rc, "fail to bulk insert, rc = %d\n", rc ) ;

done:
	return rc ;
error:
	goto done ;
}

int teardown()
{
	int rc = SDB_OK ;

	rc = db.dropCollectionSpace( CsName ) ;
	CHECK_RC( rc, "fail to drop cs %s, rc = %d\n", CsName, rc ) ;
	db.disconnect() ;

done:
	return rc ;
error:
	goto done ;
}

TEST( indexTest, createIdIndex )
{
	int rc = SDB_OK ;
	rc = setup() ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// create id index
	BSONObj option = BSON( "SortBufferSize" << 128 ) ;
	rc = cl.createIdIndex( option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create id index" ;

	// get id index
	sdbCursor cursor ;
	rc = cl.getIndexes( cursor, "$id" ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get id index" ;
		
	// query record 
	BSONObj cond = BSON( "_id" << 555 ) ;
	BSONObj sel = BSON( "_id" << "" ) ;
	BSONObj hint = BSON( "" << "$id" ) ;
	rc = cl.query( cursor, cond, sel, _sdbStaticObject, hint ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to query record" ;
	BSONObj obj ;
	rc = cursor.next( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get query cursor doc" ;
	string real = obj.toString() ;
	const char *expect = "{ \"_id\": 555 }" ;
	ASSERT_STREQ( expect, real.c_str() ) << "fail to check query result" ;
	
	// explain
	rc = cl.explain( cursor, cond, sel ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to explain query" ;
	rc = cursor.next( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get explain cursor doc" ;
	string scanType = obj.getField( "ScanType" ).String() ;
	ASSERT_STREQ( scanType.c_str(), "ixscan" ) << "fail to check scan type" ;
	string indexName = obj.getField( "IndexName" ).String() ;
	ASSERT_STREQ( indexName.c_str(), "$id" ) << "fail to check index name" ;
	
	// drop id index
	rc = cl.dropIdIndex() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop id index" ;
	
	// update after drop id index
	BSONObj rule = BSON( "$inc" << BSON( "f1" << 1 ) ) ;
    rc = cl.update( rule ) ;
    ASSERT_EQ( rc, SDB_RTN_AUTOINDEXID_IS_FALSE ) << "fail to test update after drop id index" ;

	rc = teardown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}
