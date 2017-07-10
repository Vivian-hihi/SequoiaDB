#include <client.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include "DS_common.hpp"
using namespace sdbclient ;
using namespace std ;

TEST(libTest,sdb)
{
	sdb db ;
    getConf() ;
	ASSERT_EQ(SDB_OK,db.connect(HOST, SERVER)) ;
	sdbCollectionSpace cs ;
	ASSERT_EQ(SDB_OK,db.createCollectionSpace("testcs",65536,cs)) ;
	sdbCollection cl ;
	ASSERT_EQ(SDB_OK,cs.createCollection("testcl",cl)) ;
	bson::BSONObj obj1 = BSON("a"<<"first") ;
	bson::BSONObj obj2 = BSON("b"<<"second") ;
	ASSERT_EQ(SDB_OK,cl.insert(obj1)) ;
	ASSERT_EQ(SDB_OK,cl.insert(obj2)) ;
	sdbCursor cursor ;
	bson::BSONObj sel = BSON("a"<<"") ;
	ASSERT_EQ(SDB_OK,cl.query(cursor,obj1,sel)) ;
	bson::BSONObj obj ;
	ASSERT_EQ(SDB_OK,cursor.current(obj)) ;
	ASSERT_EQ("{ \"a\": \"first\" }",obj.toString()) ;
	ASSERT_EQ(SDB_OK,cl.del(obj1)) ;
	ASSERT_EQ(SDB_OK,cl.query(cursor,obj1,sel)) ;
	ASSERT_EQ(SDB_DMS_EOC,cursor.current(obj)) ;
	ASSERT_EQ(SDB_OK,db.dropCollectionSpace("testcs")) ;
}

