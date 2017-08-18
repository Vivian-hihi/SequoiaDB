/************************************************************
* @Description: test case for Jira questionaire
*					 SEQUOIADBMAINSTREAM-837
* @Modify:		 Liang xuewang Init
*					 2017-08-18
*************************************************************/
#include <client.hpp>
#include <gtest/gtest.h>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

TEST( createNode, bson_option )
{
	int rc = SDB_OK ;
	sdb db ;
	sdbReplicaGroup tmpGroup ;
	const char* rgName = "tmpRg" ;	

	getConf() ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;

	rc = db.createReplicaGroup( rgName, tmpGroup ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create group " << rgName ;

	rc = getLocalHost() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get local host name" ;
	const char* svc = RSRVPORTBEGIN ;
	char dbpath[100] ;
	sprintf( dbpath, "%s%s", RSRVNODEDIR, svc ) ;
	BSONObj option = BSON( "weight" << 100 ) ;
	rc = tmpGroup.createNode( HOST, svc, dbpath, option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create node " << HOST << ":" << svc ;

	rc = tmpGroup.start() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to start group " << rgName ;

	rc = tmpGroup.stop() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to stop group " << rgName ;

	rc = db.removeReplicaGroup( rgName ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to remove group " << rgName ;

 	db.disconnect() ;	
}
