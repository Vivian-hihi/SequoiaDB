/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1341
* @Modify:      Liang xuewang Init
*			 	2017-07-20
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <stdlib.h>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

char *CsModName = "c_driver_test" ;
char CsName[100] ;
char *ClName = "index" ;
sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;

int setup()
{
	int rc = SDB_OK ;
	BSONObj option ;

	getConf() ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	CHECK_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ;

	getUniqueName( CsModName, CsName ) ;
	rc = db.createCollectionSpace( CsName, SDB_PAGESIZE_4K, cs ) ;
	CHECK_RC( rc, "fail to create cs %s, rc = %d\n", CsName, rc ) ;

	// option is { ShardingKey: { id: 1 } }, { ReplSize: 0 }, { Compressed: true }
	option = BSON( "ShardingKey" << BSON( "id" << 1 ) << "ReplSize" << 0
				   << "Compressed" << true ) ;
	// cout << option << endl ;

	// create cl
	rc = cs.createCollection( ClName, option, cl ) ;
	CHECK_RC( rc, "fail to create cl %s, rc = %d\n", ClName, rc ) ;

	// insert docs like { "id": 1, "f1": 2, "f2": 3 }
	for( int i = 0;i < 1000;++i )
	{
		BSONObj doc = BSON( "id" << i << "f1" << (i+1) << "f2" << (i+2) ) ;
		rc = cl.insert( doc ) ;
		CHECK_RC( rc, "fail to insert doc in the %d time, rc = %d", i, rc ) ;
	}

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

TEST( indexTest, createIndex )
{
	int rc = SDB_OK ;
	rc = setup() ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// create index
	const char *indexName = "myIndex" ;
	BSONObj indexDef = BSON( "id" << -1 ) ;
	rc = cl.createIndex( indexDef, indexName, false, false, 128 ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create index" ;

	// get index
	sdbCursor cursor ;
	rc = cl.getIndexes( cursor, indexName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get index" ;
	BSONObj obj ;
	rc = cursor.next( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get the index cursor doc" ;
	// cout << obj << endl ;
	// cout << obj.getField( "IndexDef" ).type() << endl ;
	string name = obj.getField( "IndexDef" ).Obj().getField( "name" ).String() ;
	ASSERT_STREQ( name.c_str(), indexName ) << "fail to check index name" ;
		
	// query record 
	BSONObj cond = BSON( "id" << 555 ) ;
	BSONObj sel = BSON( "id" << "" ) ;
	BSONObj hint = BSON( "" << indexName ) ;
	rc = cl.query( cursor, cond, sel, _sdbStaticObject, hint ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to query record" ;
	rc = cursor.next( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get query cursor doc" ;
	string real = obj.toString() ;
	const char *expect = "{ \"id\": 555 }" ;
	ASSERT_STREQ( expect, real.c_str() ) << "fail to check query result" ;

	rc = teardown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}
