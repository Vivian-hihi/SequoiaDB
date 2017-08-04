/**************************************************************
* @Description: test case of sync 
*				TestLink 9368  
*               ( 手工测试，测试完成后注意删除cs domain )
* @Modify     : Liang xuewang Init
*			 	2017-01-09
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/testcommon.hpp"

TEST( sdbSyncDB, normal )
{
	INT32 rc = SDB_OK ;
    sdbConnectionHandle db = SDB_INVALID_HANDLE ;
	sdbCSHandle cs         = SDB_INVALID_HANDLE ;
	sdbCollectionHandle cl = SDB_INVALID_HANDLE ;

	// connect to sdb	
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;

	// get data group and data node
	sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
    sdbNodeHandle node = SDB_INVALID_HANDLE ;
	INT32 groupId = 1000 ;
    rc = sdbGetReplicaGroup1( db, groupId, &rg ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get data group" ;
	char *rgName = NULL ;
	rc = sdbGetReplicaGroupName( rg, &rgName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get data group name" ;
	rc = sdbGetNodeSlave( rg, &node ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get node of data group" ;
	const char* hostName = NULL ;
    const char* svcName = NULL ;
    const char* nodeName = NULL ;
    INT32 nodeId = -1 ;
	rc = sdbGetNodeAddr( node, &hostName, &svcName, &nodeName, &nodeId ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get node addr" ;
	printf( "node: hostName=%s svcName=%s nodeName=%s nodeId=%d\n", hostName, svcName, nodeName, nodeId ) ;
	
	// create domain with data group
	sdbDomainHandle domain = SDB_INVALID_HANDLE ;
	const char* domainName = "syncTestDomain" ;
	bson option ;
	bson_init( &option ) ;
	bson_append_start_array( &option, "Groups" ) ;
	bson_append_string( &option, "0", rgName ) ;
	bson_append_finish_array( &option ) ;
	bson_finish( &option ) ;
	bson_print( &option ) ;
	rc = sdbCreateDomain( db, domainName, &option, &domain ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create domain" ;
	bson_destroy( &option ) ;

	// create cs cl in domain
	const char* CsModName = "C_drivertest_syncCs" ;
	char CsName[100] ;
	getUniqueName( CsModName, CsName ) ;
	bson_init( &option ) ;
	bson_append_string( &option, "Domain", domainName ) ;
	bson_finish( &option ) ;
	rc = sdbCreateCollectionSpaceV2( db, CsName, &option, &cs ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cs" ;
	bson_destroy( &option ) ;
	const char* ClModName = "C_drivertest_syncCl" ;
	char ClName[100] ;
	getUniqueName( ClModName, ClName ) ;
	rc = sdbCreateCollection( cs, ClName, &cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cl" ;

	// sync cs in data node
	bson_init( &option ) ;
	bson_append_int( &option, "Deep", 1 ) ;
	bson_append_bool( &option, "Block", false ) ;
	bson_append_string( &option, "CollectionSpace", CsName ) ;
	bson_append_bool( &option, "Global", false ) ;
	bson_append_int( &option, "GroupId", groupId ) ;
	bson_append_string( &option, "GroupName", rgName ) ;
	bson_append_int( &option, "NodeID", nodeId ) ;
	bson_append_string( &option, "HostName", hostName ) ;
	bson_append_string( &option, "svcname", svcName ) ;
	bson_finish( &option ) ;
	rc = sdbSyncDB( db, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to sync db" ;
	bson_destroy( &option ) ;
	
	rc = sdbDropCollectionSpace( db, CsName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs" ;
	rc = sdbDropDomain( db, domainName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop domain" ;
	sdbDisconnect( db ) ;
	sdbReleaseCollection( cl ) ;
	sdbReleaseCS( cs ) ;
	sdbReleaseDomain( domain ) ;
	sdbReleaseConnection( db ) ;
}
