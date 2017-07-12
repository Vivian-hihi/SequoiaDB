/********************************************************
* seqDB-11666:c驱动支持统计
* test case for sdbAnalyze
*
********************************************************/
#include <client.h>
#include <gtest/gtest.h>
#include "../common/testcommon.hpp"

const char* csModName  = "analyzeTestCs" ;
char 		csName[50] = { 0 } ;
const char* clName 	   = "analyzeTestCl" ; 
sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbCSHandle			cs = SDB_INVALID_HANDLE ;
sdbCollectionHandle cl = SDB_INVALID_HANDLE ;

// create index { a: 1 }
int createIndex( sdbCollectionHandle cl )
{
	int rc = SDB_OK ;
	bson indexDef ;
    bson_init( &indexDef ) ;

    rc = bson_append_int( &indexDef, "a", 1 ) ;
    CHECK_RC( rc, "fail to append a:1 on bson indexDef, rc = %d\n", rc ) ;
    rc = bson_finish( &indexDef ) ;
    CHECK_RC( rc, "fail to finish bson indexDef, rc = %d\n", rc ) ;
    rc = sdbCreateIndex( cl, &indexDef, "aIndex", false, false ) ;
    CHECK_RC( rc, "fail to create index in cl, rc = %d\n", rc ) ;

done:
	bson_destroy( &indexDef ) ;
	return rc ;
error:
	goto done ;
}

// insert 10000 docs { a: 100, b: "12345" }
int insertDocs( sdbCollectionHandle cl )
{
	int rc = SDB_OK ;
	int num = 10000 ;
	bson* docs[num] ;
	int i ;

    for( i = 0;i < num;++i )
    {
		docs[i] = bson_create() ;
    	rc = bson_append_int( docs[i], "a", 100 ) ;
    	CHECK_RC( rc, "fail to append a:100 on bson doc %d, rc = %d\n", i, rc ) ;
		rc = bson_append_string( docs[i], "b", "12345" ) ;
		CHECK_RC( rc, "fail to append b:12345 on bson doc %d, rc = %d\n", i, rc ) ;
    	rc = bson_finish( docs[i] ) ;
    	CHECK_RC( rc, "fail to finish bson doc %d, rc = %d\n", i, rc ) ;
	}

    rc = sdbBulkInsert( cl, 0, docs, num ) ;
    CHECK_RC( rc, "fail to insert record, rc = %d\n", rc ) ;
	for( i = 0;i < num;i++ )
	{
		bson_dispose( docs[i] ) ; 
    }

done:
	return rc ;
error:
	goto done ;
}

// query doc { a:100 } and explain
int explainDoc( sdbCollectionHandle cl, char* scanType )
{
	int rc = SDB_OK ;
	bson doc, obj ;
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
	bson_init( &doc ) ;
	bson_init( &obj ) ;
	bson_iterator it ;

	rc = bson_append_int( &doc, "a", 100 ) ;
	CHECK_RC( rc, "fail to append a:100 on bson doc, rc = %d\n", rc ) ;
	rc = bson_finish( &doc ) ;
	CHECK_RC( rc, "fail to finish bson doc, rc = %d\n", rc ) ;

    rc = sdbExplain( cl, &doc, NULL, NULL, NULL, 0, 0, -1, NULL, &cursor ) ;
    CHECK_RC( rc, "fail to explain cl with cond a:100, rc = %d\n", rc ) ;
    rc = sdbNext( cursor, &obj ) ;
    CHECK_RC( rc, "fail to get next of cursor, rc = %d\n", rc ) ;
	bson_find( &it, &obj, "ScanType" ) ;
	strcpy( scanType, bson_iterator_string( &it ) ) ;

done:
	bson_destroy( &doc ) ;
	bson_destroy( &obj ) ;
	sdbReleaseCursor( cursor ) ;
	return rc ;
error:
	goto done ;
}

TEST( analyzeTest, explain )
{
	int rc = SDB_OK ;
	
	// make unique cs name 
	getUniqueName( csModName, csName ) ;
	rc = createNormalCl( &db, &cs, &cl, csName, clName ) ;
 	ASSERT_EQ( rc, SDB_OK ) ;

	// create index { a: 1 }
	rc = createIndex( cl ) ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// insert docs
	rc = insertDocs( cl ) ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// explain query before analyze, use idx-scan
	char scanType[20] ;
	rc = explainDoc( cl, scanType ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	ASSERT_STREQ( scanType, "ixscan" ) ;

	// analyze
	bson option ;
	bson_init( &option ) ;
	rc = bson_append_string( &option, "CollectionSpace", csName ) ;
	ASSERT_EQ( rc, BSON_OK ) ;
	rc = bson_finish( &option ) ;
	ASSERT_EQ( rc, BSON_OK ) ;
	rc = sdbAnalyze( db, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to analyze cs " << csName ;
	bson_destroy( &option ) ;

	// query record after analyze, use tb-scan
	rc = explainDoc( cl, scanType ) ;
    ASSERT_EQ( rc, SDB_OK ) ;
	ASSERT_STREQ( scanType, "tbscan" ) ;

	// drop cs and release handle
	rc = sdbDropCollectionSpace( db, csName ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs " << csName ;
    sdbDisconnect( db ) ;
    sdbReleaseCollection( cl ) ;
    sdbReleaseCS( cs ) ;
    sdbReleaseConnection( db ) ;
}
