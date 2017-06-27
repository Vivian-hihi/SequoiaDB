/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-2504
* @Modify:      Liang xuewang Init
*			 	2017-06-19
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "../common/testcommon.hpp"

const char* user 	   = "" ;
const char* passwd     = "" ;
const char* csModName  = "prepareMoreTestCs" ;
char 		csName[50] = { 0 } ;
const char* clName 	   = "prepareMoreTestCl" ; 
sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbCSHandle			cs = SDB_INVALID_HANDLE ;
sdbCollectionHandle cl = SDB_INVALID_HANDLE ;

TEST( prepareMoreTest, flag_prepare_more )
{
	int rc = SDB_OK ;
	
	// make unique cs name 
	getUniqueName( csModName, csName ) ;
	rc = createNormalCl( &db, &cs, &cl, csName, clName ) ;
 	ASSERT_EQ( rc, SDB_OK ) << "fail to create cl " << clName ;

	// insert doc
	bson doc ;
	bson_init( &doc ) ;
	rc = bson_append_int( &doc, "a", 1 ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to append bson a:1" ;
	rc = bson_finish( &doc ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to finish bson" ;
	rc = sdbInsert( cl, &doc ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to insert doc to cl " << clName ;

	// query doc 
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
	rc = sdbQuery1( cl, &doc, NULL, NULL, NULL, 0,
                    -1, QUERY_PREPARE_MORE, &cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to query cl " << clName ;
	bson obj ;
	bson_init( &obj ) ;
	rc = sdbNext( cursor, &obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;
	bson_iterator it ;
	bson_find( &it, &obj, "a" ) ;
	int value = bson_iterator_int( &it ) ;
	ASSERT_EQ( value, 1 ) << "fail to check query result" ;

	// release resource
	bson_destroy( &obj ) ;
	bson_destroy( &doc ) ;
	sdbReleaseCursor( cursor ) ;
	rc = sdbDropCollectionSpace( db, csName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs " << csName ;
	sdbReleaseCollection( cl ) ;
	sdbReleaseCS( cs ) ;
	sdbReleaseConnection( db ) ;
}
