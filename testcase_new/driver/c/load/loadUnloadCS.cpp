/**************************************************************
* @Description: test case for Jira questionaire Task
*				SEQUOIADBMAINSTREAM-2165
*				seqDB-10995:unloadCS，指定的option可生效
*				seqDB-10996:unloadCS，指定的option不生效
*				seqDB-10997:loadCS，指定的option可生效
*				seqDB-10998:loadCS，指定的option不生效
* @Modify     : Liang xuewang Init
*			 	2017-01-22
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../common/testcommon.hpp"

sdbConnectionHandle db   = SDB_INVALID_HANDLE ;
sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
sdbCSHandle cs           = SDB_INVALID_HANDLE ;
sdbCollectionHandle cl   = SDB_INVALID_HANDLE ;
int groupId         = 1000 ;
const char* csModName = "loadUnloadCSTestCs" ;
char csName[100]      = ""  ;
const char* clName    = "loadUnloadCSTestCl" ;

class loadUnloadCSTest : public testing::Test
{
public:
	// run before all testcase
	static void SetUpTestCase() ;
    // run after all testcase
	static void TearDownTestCase() ;
} ;

void loadUnloadCSTest::SetUpTestCase()
{
	int rc = SDB_OK ;

	// connect sdb
    getConf() ;
    rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
    ASSERT_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ;
    if( isStandalone( db ) )
        return ;

    // get data group with groupId 1000
    rc = sdbGetReplicaGroup1( db, groupId, &rg ) ;
    ASSERT_RC( rc, "fail to get data group %d, rc = %d\n", groupId, rc ) ;
	char* rgName = NULL ;
	rc = sdbGetReplicaGroupName( rg, &rgName ) ;
	ASSERT_RC( rc, "fail to get data group name, rc = %d\n", rc ) ; 
    
	// create cs
    getUniqueName( csModName, csName ) ;
    rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, &cs ) ;
	ASSERT_RC( rc, "fail to create cs %s, rc = %d\n", csName, rc ) ;

	// create cl in data group
    bson option ;
    bson_init( &option ) ;
    bson_append_string( &option, "Group", rgName ) ;
	bson_append_int( &option, "ReplSize", 0 ) ;
	bson_finish( &option ) ;
	rc = sdbCreateCollection1( cs, clName, &option, &cl ) ;
    bson_destroy( &option ) ;
	ASSERT_RC( rc, "fail to create cl %s, rc = %d\n", clName, rc ) ;
}

void loadUnloadCSTest::TearDownTestCase()
{
	int rc = SDB_OK ;
	rc = sdbDropCollectionSpace( db, csName ) ;
	ASSERT_RC( rc, "fail to drop cs %s, rc = %d\n", csName, rc ) ;
	sdbDisconnect( db ) ;
	sdbReleaseCollection( cl ) ;
	sdbReleaseCS( cs ) ;
	sdbReleaseReplicaGroup( rg ) ;
	sdbReleaseConnection( db ) ;
}

int checkCsExist( sdbConnectionHandle db, const char* csName, bool* exist )
{
	int rc = SDB_OK ;
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
	bson obj ;
    bson_init( &obj ) ;
	*exist = false ;
    rc = sdbListCollectionSpaces( db, &cursor ) ;
    CHECK_RC( rc, "fail to list cs, rc = %d\n", rc ) ;
    while( !(rc = sdbNext( cursor, &obj)) )
    {
        bson_iterator it ;
        bson_iterator_init( &it, &obj ) ;
        const char* name = bson_iterator_string( &it ) ;
        if( !strcmp( name, csName ) )
		{
            *exist = true ;
			break ;
		}
        bson_destroy( &obj ) ;
        bson_init( &obj ) ;
    }
	rc = SDB_OK ;

done:
    bson_destroy( &obj ) ;
    sdbReleaseCursor( cursor ) ;
	return rc ;
error:
	goto done ;
}

int checkBasicOperation( sdbCollectionHandle cl )
{
	int rc = SDB_OK ;
	char clFullName[200] ;
	sprintf( clFullName, "%s.%s", csName, clName ) ;
	bson record ;
    bson_init( &record ) ;
	bson selector ;
    bson_init( &selector ) ;
	bson obj ;              
    bson_init( &obj ) ;
	bson_iterator it ;
	int result ;
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;

    // truncate cl
	rc = sdbTruncateCollection( db, clFullName ) ;
	CHECK_RC( rc, "fail to truncate cl %s, rc = %d\n", clFullName, rc ) ;

	// insert
	bson_append_int( &record, "a", 1 ) ;
	bson_finish( &record ) ;
	rc = sdbInsert( cl, &record ) ;
	CHECK_RC( rc, "fail to insert record, rc = %d\n", rc ) ;

	// query
	bson_append_string( &selector, "a", "" ) ;
	bson_finish( &selector ) ;
	rc = sdbQuery( cl, &record, &selector, NULL, NULL, 0, -1, &cursor ) ;
	CHECK_RC( rc, "fail to query record, rc = %d\n", rc ) ;

	// check query result
	rc = sdbNext( cursor, &obj ) ;
	CHECK_RC( rc, "fail to get next in cursor, rc = %d\n", rc )
	bson_iterator_init( &it, &obj ) ;
	result = bson_iterator_int( &it ) ;
	if( result != 1 )
	{
		printf( "fail to check query result,expect: %d,actual: %d\n", 1, result ) ;
		rc = SDB_DMS_RECORD_NOTEXIST ;
	}

done:
	bson_destroy( &record ) ;
    bson_destroy( &selector ) ;
	bson_destroy( &obj ) ;
	sdbReleaseCursor( cursor ) ;
	return rc ;
error:
	goto done ;
}

TEST_F( loadUnloadCSTest, validOption )
{
    int rc = SDB_OK ;
	sdbNodeHandle node = SDB_INVALID_HANDLE ;
	const char* hostName = NULL ;
    const char* svcName = NULL ;
    const char* nodeName = NULL ;
    int nodeId = -1 ;

	// check cl basic Operation
	rc = checkBasicOperation( cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to check cl basic operation in the begining" ;

	// get node hostName and svcname
    rc = sdbGetNodeSlave( rg, &node ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get slave node of group with groupID 1000" ;
    rc = sdbGetNodeAddr( node, &hostName, &svcName, &nodeName, &nodeId ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get node addr " << nodeName  ;
	printf( "node: hostName=%s svcName=%s nodeName=%s nodeId=%d\n", hostName, svcName, nodeName, nodeId ) ;

	// unload cs on node
	bson option ;
	bson_init( &option ) ;
	bson_append_string( &option, "HostName", hostName ) ;
	bson_append_string( &option, "svcname", svcName ) ;
	bson_finish( &option ) ;
	rc = sdbUnloadCollectionSpace( db, csName, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to unload cs on node " << nodeName ;

	// check unload
	sdbDisconnect( db ) ;
	sdbReleaseConnection( db ) ;
	rc = sdbConnect( hostName, svcName, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect to node after unload " << nodeName ;
	bool exist ;
	rc = checkCsExist( db, csName, &exist ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to check cs exist after unload" ;
	ASSERT_FALSE( exist ) << "fail to check unload cs on node " << nodeName ;

	// load cs on node
	sdbDisconnect( db ) ;
	sdbReleaseConnection( db ) ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb again" ;
	rc = sdbLoadCollectionSpace( db, csName, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to load cs on node " << nodeName ;

	// check load
	sdbDisconnect( db ) ;
    sdbReleaseConnection( db ) ;
    rc = sdbConnect( hostName, svcName, USER, PASSWD, &db ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to connect to node after load " << nodeName ;
    rc = checkCsExist( db, csName, &exist ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to check cs exist after load" ;
    ASSERT_TRUE( exist ) << "fail to check load cs on node " << nodeName ;

	// basic operation after load
	sdbReleaseCS( cs ) ;
	sdbReleaseCollection( cl ) ;
	rc = sdbGetCollectionSpace( db, csName, &cs ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get cs" ;
	rc = sdbGetCollection1( cs, clName, &cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get cl" ;
	rc = checkBasicOperation( cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to check basic operation after load" ;	

	// reconnect to sdb and get cs cl
	sdbDisconnect( db ) ;
	sdbReleaseConnection( db ) ;
    rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb in the end" ;
	sdbReleaseCS( cs ) ;
    sdbReleaseCollection( cl ) ;
    rc = sdbGetCollectionSpace( db, csName, &cs ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get cs in the end" ;
    rc = sdbGetCollection1( cs, clName, &cl ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get cl in the end" ;
}

TEST_F( loadUnloadCSTest, invalidOption )
{
	int rc = SDB_OK ;
	bson option ;
	bson_init( &option ) ;
	bson_append_string( &option, "GroupName", "SYSCatalogGroup" ) ;
	bson_finish( &option ) ;
	rc = sdbUnloadCollectionSpace( db, csName, &option ) ;
	ASSERT_EQ( rc, SDB_COORD_NOT_ALL_DONE ) << "fail to check unload cs on SYSCatalogGroup" ;
	rc = sdbLoadCollectionSpace( db, csName, &option ) ;
	ASSERT_EQ( rc, SDB_COORD_NOT_ALL_DONE ) << "fail to check load cs on SYSCatalogGroup" ;
}
