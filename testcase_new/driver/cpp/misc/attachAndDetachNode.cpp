/************************************************************
* @Description: test case for Jira questionaire
*               ( manual test case,not in CI ) 
*				SEQUOIADBMAINSTREAM-809
* @Modify:      Liang xuewang Init
*				2016-11-11
*************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <string>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

sdb db ;
sdbReplicaGroup dataRG, tempRG ;
sdbNode tempNode ;

int setup()
{
    int rc = SDB_OK ;
    getConf() ;
    rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
    CHECK_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ;	
done:
	return rc ;
error:
	goto done ;
}

int teardown()
{
	int rc = SDB_OK ;
    db.disconnect() ;
done:
	return rc ;
error:
	goto done ;
}

TEST( AttachAndDetachNodeTest, onlyAttachAndOnlyDetach )
{
	int rc = SDB_OK ;

	rc = setup() ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// check standalone
	if( isStandalone( db ) )
	{
		cout << "sdb connection is standalone." << endl ;
		db.disconnect() ;
		return ;
	}

	// get data group dataRG
	vector<string> groups ;
	rc = getGroups( db, groups ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	ASSERT_GT( groups.size(), 0 ) << "no data group" ;
	const char* rgname = groups[0].c_str() ;
	rc = db.getReplicaGroup( rgname, dataRG ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get rg " << rgname ;

	// create tempNode
	const char* tempNodeSvcName = RSRVPORTBEGIN ;
	cout << "temp node svcname: " << tempNodeSvcName << endl ;
	
	char tempNodeDbPath[100] ;
	sprintf( tempNodeDbPath, "%s%s", RSRVNODEDIR, tempNodeSvcName ) ;
	cout << "temp node dbpath: " << tempNodeDbPath << endl ;
    
	rc = dataRG.createNode( HOSTNAME, tempNodeSvcName, tempNodeDbPath ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create tempNode" ;
	
	// detach tempNode from dataRG
	rc = dataRG.detachNode( HOSTNAME, tempNodeSvcName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to detach tempNode" ;
	rc = dataRG.getNode( HOSTNAME, tempNodeSvcName, tempNode ) ;
	ASSERT_EQ( rc, SDB_CLS_NODE_NOT_EXIST ) << "fail to check detach" ;
	
	// create tempRG
    rc = db.createReplicaGroup( "temp", tempRG ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to create tempRG" ;
	
	// attach tempNode to tempRG
    rc = tempRG.attachNode( HOSTNAME, tempNodeSvcName ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to attach tempNode" ;
    rc = tempRG.getNode( HOSTNAME, tempNodeSvcName, tempNode ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to check attach" ;
    
	// start tempRG
    rc = tempRG.start() ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to start tempRG" ;
	
	// stop tempRG
	rc = tempRG.stop() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to stop tempRG" ;
	
	// remove tempRG
	rc = db.removeReplicaGroup( "temp" ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to remove tempRG" ;

	rc = teardown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}
