/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-2131
* @Modifyi    : Liang xuewang Init
*			 	2016-02-21
***************************************************************/
#include <iostream>
#include <client.hpp>
#include <gtest/gtest.h>
#include <stdlib.h>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace std ;
using namespace bson ;

TEST( code, procedure )
{
	int rc = SDB_OK ;
	sdb db ;

    getConf() ;
    rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;
	if( isStandalone( db ) )
	{
		cout << "Run mode is stand alone" << endl ;
		db.disconnect() ;
		return ;
	}
	
	// create procedure
	const char* func = "function add(a,b) { return a+b ; }" ;
	rc = db.crtJSProcedure( func ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create procedure" ;
	
	// list procedures
	BSONObj cond = BSON( "name" << "add" ) ;
	sdbCursor cursor ;
	rc = db.listProcedures( cursor, cond ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to list procedures" ;

	// check procedure
	BSONObj res ;
	rc = cursor.next( res ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to get next in cursor" ;
	BSONElement element = res.getField( "func" ) ;
	const char* code = element.code().c_str() ;
	ASSERT_STREQ( func, code ) << "fail to check procedure" ;
	
	// remove procedure
	rc = db.rmProcedure( "add" ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to remove procedure" ;

	// disconnect
	db.disconnect() ;
}
