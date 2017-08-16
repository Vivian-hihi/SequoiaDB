/**********************************************************
* test case for jira questionare SEQUOIADBMAINSTREAM-1518
*
**********************************************************/
#include <client.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include "../common/testcommon.hpp"

TEST( bson_array, normal )
{
	int rc = BSON_OK ;
	bson obj ;
	bson_init( &obj ) ;
	rc = bson_append_start_array( &obj, "arr" ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to append start array" ;
	rc = bson_append_string( &obj, "0", "a" ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to append first a" ;
	rc = bson_append_string( &obj, "1", "b" ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to append second b" ;
	rc = bson_append_finish_array( &obj ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to finish array" ;
	rc = bson_finish( &obj ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to finish bson" ;
	bson_print( &obj ) ;
	bson_destroy( &obj ) ;
}

TEST( bson_array, abnormal )
{
	int rc = BSON_OK ;
	bson obj ;
	bson_init( &obj ) ;
	rc = bson_append_start_array( &obj, "arr" ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to append start array" ;
	rc = bson_append_int( &obj, "", 1 ) ;
	ASSERT_EQ( rc, BSON_ERROR ) << "fail to test append array element with space" ;
	rc = bson_append_int( &obj, "a", 2 ) ;
	ASSERT_EQ( rc, BSON_ERROR ) << "fail to test append array element with a" ;
	rc = bson_append_finish_array( &obj ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to finish array" ;
	rc = bson_finish( &obj ) ;
	ASSERT_EQ( rc, BSON_OK ) << "fail to finish bson" ;
	bson_print( &obj ) ;
	bson_destroy( &obj ) ;
}

TEST( bson_array, trace )
{
	int rc = SDB_OK ;
	sdbConnectionHandle db = SDB_INVALID_HANDLE ;
   char* component = "cls, dms, mth" ;
   char* breakpoint = "engine::_dmsStorageData::_onAllocExtent" ;
   unsigned int buffSize = 10000000 ;

   getConf() ;
   rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
   rc = sdbTraceStart( db, buffSize, component, breakpoint, NULL, 0 ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to trace start" ;
   rc = sdbTraceStop( db, NULL ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to trace stop" ;

   sdbDisconnect( db ) ;
   sdbReleaseConnection( db ) ;
}

TEST( bson_array, waittask )
{
	int rc = SDB_OK ;
	sdbConnectionHandle db = SDB_INVALID_HANDLE ;
	sdbCSHandle cs = SDB_INVALID_HANDLE ;
	sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
	const char* csModName = "testWaitTaskCs" ;
	char csname[100] = { 0 } ;
	const char* clname = "testWaitTaskCl" ;

	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	
	// get data groups to be srcGroup and dstGroup
	vector<string> groups ;
	rc = getGroups( db, groups ) ;
	if( rc == SDB_RTN_COORD_ONLY )  // check standalone
	{
		printf( "Run mode is standalone.\n" ) ;
		sdbDisconnect( db ) ;
		sdbReleaseConnection( db ) ;
		return ;
	}
	ASSERT_EQ( rc, SDB_OK ) << "fail to get data groups" ;
	if( groups.size() < 2 )  // check data group num
	{
		printf( "Data group num is %d, too few.\n", groups.size() ) ;
		sdbDisconnect( db ) ;
      sdbReleaseConnection( db ) ;
		return ; 
	} 
	const char* srcGroup = groups[0].c_str() ;
	const char* dstGroup = groups[1].c_str() ;
	printf( "src group: %s, dst group: %s\n", srcGroup, dstGroup ) ;

	// create cs cl in srcGroup
	getUniqueName( csModName, csname ) ;
	rc = sdbCreateCollectionSpace( db, csname, SDB_PAGESIZE_4K, &cs ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cs " << csname ;
	bson option ;
	bson_init( &option ) ;
	bson_append_string( &option, "Group", srcGroup ) ;
	bson_append_start_object( &option, "ShardingKey" ) ;
	bson_append_int( &option, "a", 1 ) ;
	bson_append_finish_object( &option ) ;
	bson_finish( &option ) ;
	rc = sdbCreateCollection1( cs, clname, &option, &cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cl " << clname ;

	// split cl async
	long long taskID ;
	rc = sdbSplitCLByPercentAsync( cl, srcGroup, dstGroup, 50, &taskID ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to split cl async" ;
	
	// wait tasks
	long long taskIDs[1] ;
	taskIDs[0] = taskID ;
	rc = sdbWaitTasks( db, taskIDs, 1 ) ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// drop cs 
	rc = sdbDropCollectionSpace( db, csname ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs " << csname ;
	
	// disconnect, release handle, destroy bson
	sdbDisconnect( db ) ;
	bson_destroy( &option ) ;
	sdbReleaseConnection( db ) ;
	sdbReleaseCS( cs ) ;
	sdbReleaseCollection( cl ) ;
}
