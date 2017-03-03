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

const CHAR* USER   = "" ;
const CHAR* PASSWD = "" ;
sdbCollectionSpace cs ;
sdbCollection cl ;
const CHAR* csname = "lobTestCS" ;
const CHAR* clname = "lobTestCL" ;

#define CHECK_RC_CODE( rc, msg )\
do\
{\
	if( rc != SDB_OK )\
	{\
		cout << msg << endl ;\
		return rc ;\
    }\
}\
while( 0 ) ;	

INT32 createCSCL( sdb& db, sdbCollectionSpace& cs, sdbCollection& cl, const char* csName, const char* clName )
{
	INT32 rc = SDB_OK ;
	rc = db.createCollectionSpace( csName, SDB_PAGESIZE_4K, cs ) ;
    CHECK_RC_CODE( rc, "fail to create cs" )
    rc = cs.createCollection( clname, cl ) ;
	CHECK_RC_CODE( rc, "fail to create cl" )
    return rc ;
}

INT32 insertAndQuery( sdbCollection& cl, sdbCursor& cursor )
{
	INT32 rc = SDB_OK ;
	// insert and query
    BSONObj obj = BSON( "a" << "1" ) ;
    rc = cl.insert( obj ) ;
    CHECK_RC_CODE( rc, "fail to insert" )
    rc = cl.query( cursor, obj ) ;
    CHECK_RC_CODE( rc, "fail to query" )
	return rc ;
}

INT32 dropCS( sdb& db, const char* csName )
{
	INT32 rc = SDB_OK ;
	rc = db.dropCollectionSpace( csName ) ;
	CHECK_RC_CODE( rc, "fail to drop cs" ) 
	return rc ;
}

// test close all cursors with normal connection
TEST( closeAllCursors, normalConn )
{
	INT32 rc = SDB_OK ;

	// connect and create cs cl
    getConf() ;
	sdb db ;
    rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	rc = createCSCL( db, cs, cl, csname, clname ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// insert and query
	sdbCursor cursor ;
	rc = insertAndQuery( cl, cursor ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// close all cursors
	rc = db.closeAllCursors() ;
	ASSERT_EQ( rc, SDB_OK ) ;
	
	// check cursor
	BSONObj res ;
	rc = cursor.next( res ) ;
	ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check cursor" ;

	// drop cs and disconnect
	rc = dropCS( db, csname ) ;
	ASSERT_EQ( rc, SDB_OK ) ;
	db.disconnect() ; 
}

// test close all cursors with datasource connection
TEST( closeAllCursors, datasourceConn )
{
	INT32 rc = SDB_OK ;
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
	rc = ds.getConnection( conn ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get connection" ;
 	rc = createCSCL( *conn, cs, cl, csname, clname ) ;
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
    ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check cursor" ;

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
