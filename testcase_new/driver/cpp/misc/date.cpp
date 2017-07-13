/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-2507
* @Modify:      Liang xuewang Init
*			 	2017-02-28
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

const char* csName = "dateTestCs" ;
const char* clName = "dateTestCl" ;

sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;

int setup()
{
    int rc = SDB_OK ;
	rc = createNormalCl( db, cs, cl, csName,clName ) ;
	CHECK_RC( rc, "fail to create normal cl, rc = %d\n", rc ) ;
done:
	return rc ;
error:
	goto done ;
}

int teardown()
{
	int rc = SDB_OK ;
	rc = db.dropCollectionSpace( csName ) ;
	CHECK_RC( rc, "fail to drop cs %s, rc = %d\n", csName, rc ) ;
	db.disconnect() ;
done:
	return rc ;
error:
	goto done ;
}

TEST( DateTest, Date_t )
{
	int rc = SDB_OK ;
	rc = setup() ;
	ASSERT_EQ( rc, SDB_OK ) ;

	unsigned long long mills[] = {
		-62167248000000,   // 0000-01-01 00:00:00
        253402271999000,   // 9999-12-31 23:59:59
        -62167248001000,   // -0001-12-31 23:59:59 
        253402272000000    // 10000-01-01 00:00:00
	} ;
	for( int i = 0;i < sizeof(mills)/sizeof(mills[0]);i++ )
	{
		BSONObjBuilder b ;
    	BSONObj obj, res ;
    	BSONObj sel = BSON( "myDate" << "" ) ;
    	sdbCursor cursor ;
	
		rc = cl.truncate() ;
		ASSERT_EQ( rc, SDB_OK ) << "fail to truncate cl" ;

		Date_t dt( mills[i] ) ;
		b.appendDate( "myDate", dt ) ;
		obj = b.obj() ;
		
		rc = cl.insert( obj ) ;
		ASSERT_EQ( rc, SDB_OK ) << "fail to insert cl, mills: " << mills[i] ;
		
		rc = cl.query( cursor, _sdbStaticObject, sel ) ;
		ASSERT_EQ( rc, SDB_OK ) << "fail to query cl, mills: " << mills[i] ;

		rc = cursor.next( res ) ;
		ASSERT_EQ( rc, SDB_OK ) << "fail to get next, mills: " << mills[i] ;

		// cout << mills[i] << " " << res << endl ;
		BSONElement ele = res.getField( "myDate" ) ;
		BSONType type = ele.type() ;
		ASSERT_EQ( type, Date ) ;
		Date_t date = ele.Date() ;
		ASSERT_EQ( date, mills[i] ) ;
		// cout << date.toString() << endl ;
	}

	rc = teardown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}
