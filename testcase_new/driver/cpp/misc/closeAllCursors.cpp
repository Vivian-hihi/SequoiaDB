/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-2220
* @Modifyi    : Liang xuewang Init
*			 	2016-02-22
***************************************************************/
#include <iostream>
#include <client.hpp>
#include <gtest/gtest.h>
#include <sdbDataSource.hpp>
#include <sdbDataSourceComm.hpp>
#include <stdlib.h>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace std ;
using namespace bson ;

const CHAR* csname = "lobTestCS" ;
const CHAR* clname = "lobTestCL" ;

int insertAndQuery( sdbCollection& cl, sdbCursor& cursor )
{
	int rc = SDB_OK ;
	// insert and query
    BSONObj obj = BSON( "a" << "1" ) ;
    rc = cl.insert( obj ) ;
    CHECK_RC( rc, "fail to insert, rc = %d\n", rc ) ;
    rc = cl.query( cursor, obj ) ;
    CHECK_RC( rc, "fail to query, rc = %d\n", rc ) ;
done:
	return rc ;
error:
	goto done ;
}

int dropCS( sdb& db, const char* csName )
{
	int rc = SDB_OK ;
	rc = db.dropCollectionSpace( csName ) ;
	CHECK_RC( rc, "fail to drop cs %s, rc = %d\n", csName, rc ) ;
done: 
	return rc ;
error:
	goto done ;
}

// test close all cursors with normal connection
TEST( closeAllCursors, normalConn )
{
	int rc = SDB_OK ;

	// connect and create cs cl
	sdb db ;
	sdbCollectionSpace cs ;
	sdbCollection cl ;
	rc = createNormalCl( db, cs, cl, csname, clname ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// insert and query
	rc = insertAndQuery( cl, cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// close all cursors
	rc = db.closeAllCursors() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close all cursors" ;
	
	// check cursor
	BSONObj res ;
	rc = cursor.next( res ) ;
	ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check cursor after close all cursors" ;

	// drop cs and disconnect
	rc = dropCS( db, csname ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	db.disconnect() ; 
}

// test close all cursors with datasource connection
TEST( closeAllCursors, datasourceConn )
{
	int rc = SDB_OK ;
	sdbDataSource ds ;
	sdbDataSourceConf conf ;

	// init data source
	getConf() ;
	conf.setUserInfo( USER, PASSWD ) ;
	char tmp[100] ;
	sprintf( tmp, "%s%s%s", HOSTNAME, ":", SVCNAME ) ;
	string url( tmp ) ;
	rc = ds.init( url, conf ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to init data source" ;
	rc = ds.enable() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to enable data source" ;

	// get connection and create cs cl
	sdb* conn ;
	sdbCollectionSpace cs ;
	sdbCollection cl ;
	rc = ds.getConnection( conn ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get connection" ;
 	rc = createNormalCl( *conn, cs, cl, csname, clname ) ;
	ASSERT_EQ( rc, SDB_OK ) ;

	// insert and query
	sdbCursor cursor ;
	rc = insertAndQuery( cl, cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// close all cursors with release connection
	ds.releaseConnection( conn ) ;

	// check cursor
	BSONObj res ;
    rc = cursor.next( res ) ;
    ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check cursor after release connection" ;

    // get connection again and drop cs
	rc = ds.getConnection( conn ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to get connection again" ;
    rc = dropCS( *conn, csname ) ;
    ASSERT_EQ( rc, SDB_OK ) ;
    
	// disable and close data source
	rc = ds.disable() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to disable data source" ;
	ds.close() ;
}
