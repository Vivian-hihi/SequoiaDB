/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-2248
* @Modifyi    : Liang xuewang Init
*			 	2016-02-17
***************************************************************/
#include <iostream>
#include <client.hpp>
#include <gtest/gtest.h>
#include <stdlib.h>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace std ;
using namespace bson ;

sdb db ;
sdbCollectionSpace cs ;
sdbCollection cl ;
const char* csname = "lobTestCS" ;
const char* clname = "lobTestCL" ;

class LobTest : public testing::Test
{
public:
	static void SetUpTestCase() ;
	static void TearDownTestCase() ;	
} ;

void LobTest::SetUpTestCase()
{
	int rc = SDB_OK ;
    rc = createNormalCl( db, cs, cl, csname, clname ) ;
    ASSERT_RC( rc, "fail to create normal cl, rc = %d\n", rc ) ;
}

void LobTest::TearDownTestCase()
{
	int rc = SDB_OK ;
	rc = db.dropCollectionSpace( csname ) ;
	ASSERT_RC( rc, "fail to drop cs %s, rc = %d\n", csname, rc ) ;
}

// creat lob then close all cursors
TEST_F( LobTest, create )
{
	int rc = SDB_OK ;
	sdbLob lob ;
	OID oid1, oid2 ;
	UINT64 time1, time2 ;
	SINT64 size1, size2 ;
	char buf[10] = { 0 } ;

	// create lob
	rc = cl.createLob( lob ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create lob" ;

	// get oid, time, size
	oid1 = lob.getOid() ;
	time1 = lob.getCreateTime() ;
	size1 = lob.getSize() ;

	// close all cursors
	rc = db.closeAllCursors() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close all cursors" ;

	// write lob
	rc = lob.write( buf, 10 ) ;
	ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check write lob after close all cursors" ;

	// close lob
	rc = lob.close() ;
  	ASSERT_EQ( rc, SDB_OK ) << "fail to close lob" ;
	rc = lob.close() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close lob again" ;
   	BOOLEAN flag = FALSE ;
   	rc = lob.isClosed( flag ) ;
   	ASSERT_EQ( TRUE, flag ) << "fail to check lob closed" ;

   	// get oid/create time/lob size
   	oid2  = lob.getOid() ;
   	time2 = lob.getCreateTime() ;
   	size2 = lob.getSize() ;

   	ASSERT_EQ( 0, oid1.compare( oid2 ) ) << "fail to check lob oid " << oid1.toString() << " " << oid2.toString() ;
   	ASSERT_EQ( time1, time2 ) << "fail to check lob create time" ;
   	ASSERT_EQ( size1, size2 ) << "fail to check lob size" ;
}

// write lob then close all cursors
TEST_F( LobTest, write )
{
	int rc = SDB_OK ;
    sdbLob lob ;
    OID oid1, oid2 ;
    UINT64 time1, time2 ;
    SINT64 size1, size2 ;
    char buf[10] = { 0 } ;

    // create lob
    rc = cl.createLob( lob ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to create lob" ;

    // get oid, time, size
    oid1 = lob.getOid() ;
    time1 = lob.getCreateTime() ;
    size1 = lob.getSize() ;
	
	// write lob
	rc = lob.write( buf, 10 ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to write lob" ;
	size1 += 10 ;

    // close all cursors
    rc = db.closeAllCursors() ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to close all cursors" ;

    // write lob
    rc = lob.write( buf, 10 ) ;
    ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check write lob after close all cursors" ;

    // close lob
    rc = lob.close() ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;
    BOOLEAN flag = FALSE ;
    rc = lob.isClosed( flag ) ;
    ASSERT_EQ( TRUE, flag ) << "fail to check lob closed" ;

	// get oid/create time/lob size
    oid2  = lob.getOid() ;
    time2 = lob.getCreateTime() ;
    size2 = lob.getSize() ;

    ASSERT_EQ( 0, oid1.compare( oid2 ) ) << "fail to check lob oid " << oid1.toString() << " " << oid2.toString() ;
    ASSERT_EQ( time1, time2 ) << "fail to check lob create time" ;
    ASSERT_EQ( size1, size2 ) << "fail to check lob size" ;
}

// read lob then close all cursors
TEST_F( LobTest, read )
{
	int rc = SDB_OK ;
    sdbLob lob ;
    OID oid1, oid2 ;
    UINT64 time1, time2 ;
    SINT64 size1, size2 ;
    char buf[100] = { 0 } ;

    // create lob
    rc = cl.createLob( lob ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to create lob" ;
    // get oid
    oid1 = lob.getOid() ;
    // write lob
    rc = lob.write( buf, 100 ) ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to write lob" ;
	// close lob
    rc = lob.close() ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;

	// read lob	
	rc = cl.openLob( lob, oid1 ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to open lob" ;
	time1 = lob.getCreateTime() ;
	size1 = lob.getSize() ;
	char readBuf[100] = {} ;
	UINT32 len = 10 ;
	UINT32 readLen ;
	rc = lob.read( len, readBuf, &readLen ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to read lob" ;
	ASSERT_EQ( len, readLen ) << "fail to check lob read len" ;

    // close all cursors
    rc = db.closeAllCursors() ;
    ASSERT_EQ( rc, SDB_OK ) << "fail to close all cursors" ;

    // read lob again
	rc = lob.seek( 10, SDB_LOB_SEEK_CUR ) ;
    ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check seek after close all cursors" ;
    rc = lob.read( len, readBuf, &readLen ) ;
    ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check read after close all cursors" ;

    // close lob
    rc = lob.close() ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to close lob" ;

    // get oid/create time/lob size
    oid2  = lob.getOid() ;
    time2 = lob.getCreateTime() ;
    size2 = lob.getSize() ;

    ASSERT_EQ( 0, oid1.compare( oid2 ) ) << "fail to check lob oid " << oid1.toString() << " " << oid2.toString() ;
    ASSERT_EQ( time1, time2 ) << "fail to check lob create time" ;
    ASSERT_EQ( size1, size2 ) << "fail to check lob size" ;
}

// query then close all cursors
TEST_F( LobTest, query )
{
	int rc = SDB_OK ;

    // insert and query
	BSONObj obj = BSON( "a" << "1" ) ;
	rc = cl.insert( obj ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to insert obj" ;
	BSONObj sel = BSON( "a" << "" ) ;
	sdbCursor cursor ;
	rc = cl.query( cursor, obj, sel ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to query cl" ;

	// close all cursors
	rc = db.closeAllCursors() ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to close all cursors" ;

	// check cursor is closed
	BSONObj res ;
	rc = cursor.next( res ) ;
	ASSERT_EQ( rc, SDB_DMS_CONTEXT_IS_CLOSE ) << "fail to check cursor after close all cursors" ;
}
