/**************************************************************
* @Description: test case of sync 
*				TestLink 12237
* @Modify     : Liang xuewang Init
*			 	2017-01-09
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

TEST( syncDB, normal )
{
	int rc = SDB_OK ;

	// connect to sdb	
	getConf() ;
	sdb db ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;

	// get all data groups
	vector<string> groups ;
	rc = getGroups( db, groups ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	ASSERT_GT( groups.size(), 0 ) << "no data group" ;

	// get group name and group id
	const char* rgName = groups[0].c_str() ;
	sdbReplicaGroup rg ;
	rc = db.getReplicaGroup( rgName, rg ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get rg " << rgName ;
	BSONObj detail ;
	rc = rg.getDetail( detail ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get rg detail" ;
	int groupId = detail.getField( "GroupID" ).Int() ;
	cout << "group: name = " << rgName << ", id = " << groupId << endl ;
		
	// get slave data node 
    sdbNode node ;
	rc = rg.getSlave( node ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get slave node" ;
	const char* hostName = node.getHostName() ;
    const char* svcName = node.getServiceName() ;
    const char* nodeName = node.getNodeName() ;
	cout << "node: hostname = " << hostName << ", svcName = " << svcName 
		 << ", nodeName = " << nodeName << endl ;

	// create cs cl in group
	const char* CsModName = "Cpp_drivertest_syncCs" ;
	char CsName[100] ;
	getUniqueName( CsModName, CsName ) ;
	sdbCollectionSpace cs ;
	rc = db.createCollectionSpace( CsName, SDB_PAGESIZE_4K, cs ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cs " << CsName ;
	const char* ClName = "Cpp_drivertest_syncCl" ;
	sdbCollection cl ;
	BSONObj option = BSON( "Group" << rgName ) ;
	rc = cs.createCollection( ClName, option, cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cl " << ClName ;

	// sync with no option
	rc = db.syncDB() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to sync with no option" ;

	// sync with option
	BSONObj syncOption = BSON( "Deep" << 1 << "Block" << false << "CollectionSpace" << CsName
							   << "Global" << false << "GroupId" << groupId 
							   << "GroupName" << rgName << "HostName" << hostName 
							   << "svcname" << svcName ) ;
	rc = db.syncDB( option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to sync with option" ;
	
	// drop cs and disconnect
	rc = db.dropCollectionSpace( CsName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs " << CsName ;
	db.disconnect() ;
}

TEST( syncDB, abnormal )
{
	int rc = SDB_OK ;

	sdb db ;
	getConf() ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	
	// sync with invalid option
	BSONObj option = BSON( "HostName" << "InvalidHost" ) ;
	rc = db.syncDB( option ) ;
	ASSERT_EQ( rc, SDB_CLS_NODE_NOT_EXIST ) ;
}
