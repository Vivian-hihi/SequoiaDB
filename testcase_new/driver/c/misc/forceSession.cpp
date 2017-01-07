/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-2148
* @Modify     : Liang xuewang Init
*			 	2017-01-06
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/testcommon.hpp"

#define CHECK_RC_CODE( rc, msg )\
do\
{\
   if( SDB_OK != rc )\
   {\
      printf( "%s[%d]: %s, rc = %d\n", __FILE__, __LINE__, msg, rc ) ;\
      return rc ; \
   }\
}\
while( 0 ) ;

INT32 getCurrentSessionId( sdbConnectionHandle db, SINT64* sessionId )
{
    INT32 rc = SDB_OK ;
    // list current session
	sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
    bson condition ;
    bson_init( &condition ) ;
    bson_append_bool( &condition, "Global", false ) ;
    bson_finish( &condition ) ;
    bson selector ;
    bson_init( &selector ) ;
    bson_append_string( &selector, "SessionID", "" ) ;
    bson_finish( &selector ) ;
    rc = sdbGetList( db, SDB_LIST_SESSIONS_CURRENT, &condition, &selector, NULL, &cursor ) ;
    CHECK_RC_CODE( rc, "fail to list current session" )
	bson_destroy( &condition ) ;
	bson_destroy( &selector ) ;

	// get session id
    bson obj ;
    bson_init( &obj ) ;
    rc = sdbNext( cursor, &obj ) ;
    CHECK_RC_CODE( rc, "fail to get next in listCurrentSession cursor" )
    bson_iterator it ;
    bson_iterator_init( &it, &obj ) ;
    *sessionId = bson_iterator_int( &it ) ;
	bson_destroy( &obj ) ;	
	sdbReleaseCursor( cursor ) ;
}

INT32 getNodeSessionIds( sdbConnectionHandle db, const char* nodeName, SINT64 sessionId[], int size )
{
	INT32 rc = SDB_OK ;
	// get node sessions
    bson condition ;
    bson_init( &condition ) ;
    bson_append_string( &condition, "NodeName", nodeName ) ;
    bson_append_bool( &condition, "Global", false ) ;
    bson_finish( &condition ) ;
    bson selector ;
    bson_init( &selector ) ;
    bson_append_string( &selector, "SessionID", "" ) ;
    bson_finish( &selector ) ;
    sdbCursorHandle cursor = SDB_INVALID_HANDLE ;
    rc = sdbGetList( db, SDB_LIST_SESSIONS, &condition, &selector, NULL, &cursor ) ;
    CHECK_RC_CODE( rc, "fail to list sessions on node" )
    bson_destroy( &condition ) ;
    bson_destroy( &selector ) ;
    // get session ids
    bson obj ;
    bson_init( &obj ) ;
    int i = 0 ;
    while( !( rc = sdbNext( cursor, &obj ) ) && i < size )
    {
        bson_iterator it ;
        bson_iterator_init( &it, &obj ) ;
        sessionId[i++] = bson_iterator_int( &it ) ;
        bson_destroy( &obj ) ;
        bson_init( &obj ) ;
    }
    bson_destroy( &obj ) ;
    sdbReleaseCursor( cursor ) ;
}

TEST( forceSession, currentSession )
{
	INT32 rc = SDB_OK ;
    sdbConnectionHandle db = SDB_INVALID_HANDLE ;
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
    if( isStandalone( db ) )
		return ;
	SINT64 oldSessionId = -1 ;
	SINT64 newSessionId = -1 ;
	int groupId[] = { 1, 2, 1000 } ;   // catalogRG/coordRG/dataRG

	const char* hostName = NULL ;
    const char* svcName = NULL ;
    const char* nodeName = NULL ;
    INT32 nodeId = -1 ;

	for( int i = 0;i < sizeof(groupId)/sizeof(groupId[0]);i++ )
	{
    	// get node addr and connect
    	sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
    	sdbNodeHandle node = SDB_INVALID_HANDLE ;
    	rc = sdbGetReplicaGroup1( db, groupId[i], &rg ) ;
    	ASSERT_EQ( rc, SDB_OK ) << "fail to get group " << groupId[i] ;
    	rc = sdbGetNodeSlave( rg, &node ) ;
    	ASSERT_EQ( rc, SDB_OK ) << "fail to get slave node of group " << groupId[i] ;
    	rc = sdbGetNodeAddr( node, &hostName, &svcName, &nodeName, &nodeId ) ;
    	ASSERT_EQ( rc, SDB_OK ) << "fail to get node addr " << nodeName  ;
		printf( "node: hostName=%s svcName=%s nodeName=%s nodeId=%d\n", hostName, svcName, nodeName, nodeId ) ;
    	sdbDisconnect( db ) ;
		sdbReleaseConnection( db ) ;
    	rc = sdbConnect( hostName, svcName, USER, PASSWD, &db ) ;
    	ASSERT_EQ( rc, SDB_OK ) << "fail to connect node" ;

    	// force catalog node current session
    	rc = getCurrentSessionId( db, &oldSessionId ) ;
    	ASSERT_EQ( rc, SDB_OK ) ;
    	rc = sdbForceSession( db, oldSessionId, NULL ) ;
    	ASSERT_EQ( rc, SDB_NETWORK_CLOSE ) << "fail to test force curent session" ;
    	
		// reconect and check session id
		sdbDisconnect( db ) ;
        sdbReleaseConnection( db ) ;
    	rc = sdbConnect( hostName, svcName, USER, PASSWD, &db ) ;
    	ASSERT_EQ( rc, SDB_OK ) << "fail to connect node after force session" ;
    	rc = getCurrentSessionId( db, &newSessionId ) ;
    	ASSERT_EQ( rc, SDB_OK ) ;
    	ASSERT_NE( newSessionId, oldSessionId ) << "fail to check session id" ;
		
        // reconect to coord
    	sdbDisconnect( db ) ;
    	sdbReleaseConnection( db ) ;
		rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
    	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	}
	
	sdbDisconnect( db ) ;
    sdbReleaseConnection( db ) ;
}

TEST( forceSession, withOption )
{
	INT32 rc = SDB_OK ;
    sdbConnectionHandle db = SDB_INVALID_HANDLE ;
    rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	SINT64 oldSessionId = -1 ;
    SINT64 newSessionId = -1 ;

	const char* groupName = "SYSCatalogGroup" ;
	const int groupId = 1 ;
	const char* hostName = NULL ;
	const char* svcName = NULL ;
	const char* nodeName = NULL ;
	INT32 nodeId = -1 ;
	
	// get node addr and connect
    sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
    sdbNodeHandle node = SDB_INVALID_HANDLE ;
    rc = sdbGetReplicaGroup1( db, groupId, &rg ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get SYSCoord group" ;
    rc = sdbGetNodeSlave( rg, &node ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get slave node of group " ;
    rc = sdbGetNodeAddr( node, &hostName, &svcName, &nodeName, &nodeId ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get node addr " << nodeName  ;
    sdbDisconnect( db ) ;
    sdbReleaseConnection( db ) ;
    rc = sdbConnect( hostName, svcName, USER, PASSWD, &db ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to connect node " << nodeName ;
	printf( "node: hostName=%s svcName=%s nodeName=%s nodeId=%d\n", hostName, svcName, nodeName, nodeId ) ;

	// get session ids on node	
	SINT64 sessionIds[1024] ;
	memset( sessionIds, 0, sizeof(sessionIds) ) ;
	rc = getNodeSessionIds( db, nodeName, sessionIds, 1024 ) ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// get last session id to force
	SINT64 sessionId ;
	for( int i = 0;i < 1024 && sessionIds[i] != 0;i++ )
	{
		sessionId = sessionIds[i] ;
	}
	
	// make option
	bson option ;
	bson_init( &option ) ;
	bson_append_string( &option, "HostName", hostName ) ;
	bson_append_string( &option, "svcname", svcName ) ;
	bson_append_int( &option, "NodeID", nodeId ) ;
	bson_append_int( &option, "GroupID", groupId ) ;
	bson_append_string( &option, "GroupName", groupName ) ;
	bson_finish( &option ) ;
	bson_print( &option ) ;	

	// force session with db connected to coord
	sdbDisconnect( db ) ;
	sdbReleaseConnection( db ) ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	rc = sdbForceSession( db, sessionId, &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to force session " << sessionId ;
	
	// check
	memset( sessionIds, 0, sizeof(sessionIds) ) ;
	sdbDisconnect( db ) ;
    sdbReleaseConnection( db ) ;
    rc = sdbConnect( hostName, svcName, USER, PASSWD, &db ) ;
    rc = getNodeSessionIds( db, nodeName, sessionIds, 1024 ) ;
    ASSERT_EQ( rc, SDB_OK ) ;
	bool found = false ;
	for( int i = 0;i < 1024 && sessionIds[i] != 0;i++ )
    {
        if( sessionId == sessionIds[i] )
		{ 
			found = true ;
			break ;
		}
    }
	ASSERT_FALSE( found ) ;
	
	sdbDisconnect( db ) ;
    sdbReleaseConnection( db ) ;		 
}	
