/**************************************************************
* @Description: test case for Jira questionaire Task
*				SEQUOIADBMAINSTREAM-2165
*				seqDB-11001:reloadConf
*               修改数据组备节点的配置文件weight=20
*				重新选主，检查备节点升级为主节点
* @Modify     : Liang xuewang Init
*			 	2017-01-22
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include "../common/testcommon.hpp"

#define CHECK_RC_CODE( rc, msg ) \
if( rc != SDB_OK ) \
{ \
	printf( "%s,rc = %d\n", msg, rc ) ; \
	return rc ; \
}

sdbConnectionHandle db = SDB_INVALID_HANDLE ;

char* getInstallPath( char* path )
{
	const char* installFile = "/etc/default/sequoiadb" ;
	FILE* fp = fopen( installFile, "r" ) ;
	if( fp == NULL )
	{
		printf( "fail to open file /etc/default/sequoiadb\n" ) ;
		exit(1) ;
	}
	char s[50] ;
	while( fgets(s,sizeof(s),fp) != NULL  )
	{
		char* idx ;
		if( (idx=strstr(s,"INSTALL_DIR=")) == NULL )
			continue ;
		strcpy( path, idx+12 ) ;
		break ;
	}
	fclose( fp ) ;
	int len = strlen(path) ;
	path[len-1] = '\0' ;
	if( strcmp(path, "") == 0 )
	{
		printf( "fail to get install path\n" ) ;
		exit(1) ;
	}
	
	return path ;
}

INT32 isMasterNode( sdbReplicaGroupHandle& rg, sdbNodeHandle& node, bool* res )
{
	INT32 rc = SDB_OK ;

    const char *host, *svc, *nodename ;
    INT32 nodeId ;
    rc = sdbGetNodeAddr( node, &host, &svc, &nodename, &nodeId ) ;
    CHECK_RC_CODE( rc, "fail to get slave node addr in function isMasterNode" ) ;

    sdbNodeHandle master ;
    rc = sdbGetNodeMaster( rg, &master ) ;
    CHECK_RC_CODE( rc, "fail to get master node in function isMasterNode" ) ;
    const char *host1, *svc1, *nodename1 ;
    INT32 nodeId1 ;
    rc = sdbGetNodeAddr( master, &host1, &svc1, &nodename1, &nodeId1 ) ;
    CHECK_RC_CODE( rc, "fail to get master node addr in function isMasterNode" ) ;

    if( strcmp(host, host1) == 0 && strcmp(svc, svc1) == 0 )
    	*res = true ;
	else
		*res = false ;

	sdbReleaseNode( master ) ;
	return rc ;
}

bool isLocalHost( const char* host )
{
	getHost() ;
    if( strcmp(host, "localhost") != 0 && strcmp(host, HOST) != 0 )
    	return false ;
	else
		return true ;
}

INT32 restartNode( sdbNodeHandle& node )
{
	INT32 rc = SDB_OK ;
	
	rc = sdbStopNode( node ) ;
    CHECK_RC_CODE( rc, "fail to stop node" ) ;
    rc = sdbStartNode( node ) ;
    CHECK_RC_CODE( rc, "fail to start node" ) ;	

	return rc ;
}

INT32 printNode( sdbNodeHandle& node )
{
	INT32 rc = SDB_OK ;

    const char *host, *svc, *nodename ;
    INT32 nodeId ;
    rc = sdbGetNodeAddr( node, &host, &svc, &nodename, &nodeId ) ;
    CHECK_RC_CODE( rc, "fail to get slave node addr in function printNode" ) ;

	printf( "node: name %s,svc %s,nodename %s,nodeId %d\n", host, svc, nodename, nodeId ) ;

    return rc ;
}

// get a slave data node which is on the same machine with coord
INT32 getSlaveNode( sdbReplicaGroupHandle& rg, sdbNodeHandle& node )
{
	INT32 rc = SDB_OK ;
	bool found = false ;
	
	// list rg
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
	rc = sdbListReplicaGroups( db, &cursor ) ;
	CHECK_RC_CODE( rc, "fail to list rg" ) ;
	bson obj ;	
	bson_init( &obj ) ;
	while( sdbNext( cursor, &obj ) == SDB_OK )
	{
		bson_iterator it ;
		bson_find( &it, &obj, "GroupName" ) ;
		const char* rgname = bson_iterator_string( &it ) ;
		if( strcmp(rgname, "SYSCoord") == 0 || strcmp(rgname, "SYSCatalogGroup") == 0 )
		{
			bson_destroy( &obj ) ;
			bson_init( &obj ) ;
			continue ;
		}
		bson_find( &it, &obj, "Group" ) ;
		bson_print( &obj ) ;

		bson_iterator sub_it ;
		bson_iterator_subiterator( &it, &sub_it ) ;
		while( bson_iterator_next( &sub_it ) )
		{
			bson sub_obj ;
			bson_init( &sub_obj ) ;
			bson_iterator_subobject( &sub_it, &sub_obj ) ;
			bson_print( &sub_obj ) ;
   
			bson_iterator sub_sub_it ;
			bson_find( &sub_sub_it, &sub_obj, "HostName" ) ;
			const char* host = bson_iterator_string( &sub_sub_it ) ;
			if( !isLocalHost( host ) )
			{
				bson_destroy( &sub_obj ) ;
				continue ;
			}

			bson_find( &sub_sub_it, &sub_obj, "Service" ) ;
			bson_iterator tmp ;
			bson_iterator_subiterator( &sub_sub_it, &tmp ) ;
			bson_iterator_next( &tmp ) ;
			bson b ;
			bson_init( &b ) ;
			bson_iterator_subobject( &tmp, &b ) ;
			bson_find( &tmp, &b, "Name" ) ;
			bson_print( &b ) ;
			const char* svc = bson_iterator_string( &tmp ) ;

			rc = sdbGetReplicaGroup( db, rgname, &rg ) ;
        	CHECK_RC_CODE( rc, "fail to get rg" ) ;			
			rc = sdbGetNodeByHost( rg, host, svc, &node ) ;	
			CHECK_RC_CODE( rc, "fail to get node" ) ;		
	
			bson_destroy( &b ) ;
			bson_destroy( &sub_obj ) ;
			found = true ;
			break ;
		}
		if( found ) break ;
	}
	
	bool isMaster = false ;
	rc = isMasterNode( rg, node, &isMaster ) ;
	CHECK_RC_CODE( rc, "fail to check master node in function getSlaveNode" ) ;
	if( isMaster )
	{		
	    rc = restartNode( node ) ;
	    CHECK_RC_CODE( rc, "fail to restart master node" ) ;
	}

	rc = printNode( node ) ;
    CHECK_RC_CODE( rc, "fail to print node" ) ;

	bson_destroy( &obj ) ;
	sdbReleaseCursor( cursor ) ;
	return rc ;
}

INT32 changeNodeConf( sdbNodeHandle& node, const char* conf, int value )
{
	INT32 rc = SDB_OK ;

	char installPath[20] ;
	getInstallPath( installPath ) ;

	const char *host, *svc, *nodename ;
	INT32 nodeId ;
	rc = sdbGetNodeAddr( node, &host, &svc, &nodename, &nodeId ) ;
	CHECK_RC_CODE( rc, "fail to get node addr" ) ;
	
	char confFile[100] ;
	sprintf( confFile, "%s%s%s%s", installPath, "/conf/local/", svc, "/sdb.conf" ) ;
	FILE* fp = fopen( confFile, "r+" ) ;
	if( fp == NULL )
	{
		printf( "fail to open conf file: %s\n", confFile ) ;
		exit(1) ;
	}
	
	char buffer[100] ;
	sprintf( buffer, "%s%s%d", conf, "=", value ) ;
	char s[100] ;
	int len = 0 ;
	while( fgets(s,sizeof(s),fp) != NULL )
	{
		len += strlen(s) ;
		char* idx ;
		if( (idx=strstr(s,conf)) != NULL )
		{
			len -= strlen(s) ;
        	break ;
		}
	}
	if( fseek(fp,len,SEEK_SET) != 0 )
	{
		printf( "fail to seek file,file: %s,offset: %d\n", confFile, len ) ;
		exit(1) ;
	}
	fprintf( fp, "%s", buffer ) ;
	fclose( fp ) ;

	return rc ;
}

TEST( reloadConf, weight )
{
	INT32 rc = SDB_OK ;

    // connect to sdb
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	if( isStandalone(db) ) return ;

	// get slave node
	sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
	sdbNodeHandle node = SDB_INVALID_HANDLE ;
	rc = getSlaveNode( rg, node ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// change slave node weight to 20
	rc = changeNodeConf( node, "weight", 20 ) ;
	ASSERT_EQ( rc, SDB_OK ) ;

 	// reload conf
    rc = sdbReloadConfig( db, NULL ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to reload conf" ;

	// reelect and check master
	rc = sdbReelect( rg, NULL ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to reelect in rg" ;
	bool isMaster = false ;
	rc = isMasterNode( rg, node, &isMaster ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to check node is master node or not" ;
	ASSERT_TRUE( isMaster ) << "fail to check node to be master after reelect" ;	

	// change slave node weight to 10(default) after test
	rc = changeNodeConf( node, "weight", 10 ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// reload conf again
    rc = sdbReloadConfig( db, NULL ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to reload conf again" ;

	// disconnect and release
	sdbDisconnect( db ) ;
	sdbReleaseConnection( db ) ;
	sdbReleaseReplicaGroup( rg ) ;
	sdbReleaseNode( node ) ;
}
