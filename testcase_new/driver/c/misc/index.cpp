/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1341
* @Modify:      Liang xuewang Init
*			 	2016-11-10
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include "../common/testcommon.hpp"

char *CsModName = "c_driver_test" ;
char CsName[100] ;
char *ClName = "index" ;
sdbConnectionHandle db = 0 ;
sdbCSHandle cs = 0 ;
sdbCollectionHandle cl = 0 ;

void prepareCl()
{
	int rc = SDB_OK ;
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to connect sdb" ;
	getUniqueName( CsModName,CsName ) ;
	rc = sdbCreateCollectionSpace(db,CsName,SDB_PAGESIZE_4K,&cs) ;
	if( rc == SDB_DMS_CS_EXIST )
	{
		rc = sdbDropCollectionSpace( db, CsName ) ;
		ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs existed" ;
	    rc = sdbCreateCollectionSpace(db,CsName,SDB_PAGESIZE_4K,&cs) ;	
	}
	ASSERT_EQ(rc,SDB_OK)<<"fail to create cs" ;
	// option is {ShardingKey:{id:1}},{ReplSize:0},{Compressed:true}
	bson options ;
	bson_init(&options) ;
	bson_append_start_object(&options,"ShardingKey") ;
	bson_append_int(&options,"id",1) ;
	bson_append_finish_object(&options) ;
	bson_append_int(&options,"ReplSize",0) ;
	bson_append_bool(&options,"Compressed",true) ;
	bson_finish(&options) ;
	// bson_print(&options) ;
	// create cl
	rc = sdbCreateCollection1(cs,ClName,&options,&cl) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to create cl" ;
	// destroy options bson
	bson_destroy(&options) ;
	// insert records like {"id":1,"f1":2,"f2":3}
	for(int i=0;i<1000;++i)
	{
		bson record ;
		bson_init(&record) ;
		bson_append_int(&record,"id",i) ;
		bson_append_int(&record,"f1",i+1) ;
		bson_append_int(&record,"f2",i+2) ;
		bson_finish(&record) ;
		// bson_print(&record) ;
		rc = sdbInsert(cl,&record) ;
		ASSERT_EQ(rc,SDB_OK)<<"fail to insert record in the "<<i<<" times" ;
		bson_destroy(&record) ;
	}
}

void cleanResource()
{
	int rc = SDB_OK ;
	// drop cs cl disconnect
	rc = sdbDropCollection(cs,ClName) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to drop cl" ;
	rc = sdbDropCollectionSpace(db,CsName) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to drop cs" ;
	sdbDisconnect(db) ;
	// release handle
	sdbReleaseCollection(cl) ;
	sdbReleaseCS(cs) ;
	sdbReleaseConnection(db) ;
}

TEST(indexTest,createIndex)
{
	// prepare:create cl and insert records
	prepareCl() ;

	// create index
	int rc = SDB_OK ;
	char *indexName = "myIndex" ;
	bson indexDef ;
	bson_init(&indexDef) ;
	bson_append_int(&indexDef,"id",-1) ;
	bson_finish(&indexDef) ;
	rc = sdbCreateIndex1(cl,&indexDef,indexName,false,false,128) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to create index" ;
	bson_destroy(&indexDef) ;

	// get index
	sdbCursorHandle cursor ;
	rc = sdbGetIndexes(cl,indexName,&cursor) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to get index" ;
	bson obj ;
	bson_init(&obj) ;
	rc = sdbNext(cursor,&obj) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to get the index cursor doc" ;
	bson_iterator it ;
	bson_iterator_init(&it,&obj) ;
	bson_iterator sub ;
	bson_iterator_subiterator(&it,&sub) ;
	const char *name = bson_iterator_string(&sub) ;
	ASSERT_EQ(0,strcmp(name,indexName))<<"index name wrong,expect:"<<indexName<<" actual:"<<name ;
	sdbReleaseCursor( cursor ) ;
		
	// query record 
	const char* c = "{id:555}" ;
	bson cond ;
	jsonToBson(&cond,c) ;
	const char* s = "{id:\"\"}" ;
	bson sel ;
	jsonToBson(&sel,s) ;
	const char* h = "{id:0}" ;
	bson hint ;
	jsonToBson(&hint,h) ;
	rc = sdbQuery(cl,&cond,&sel,NULL,&hint,0,-1,&cursor) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to query record" ;
	rc = sdbNext(cursor,&obj) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to get query cursor doc" ;
	char result[100] = {0} ;
	bson_sprint(result,sizeof(result),&obj) ;
	char *expect = "{ \"id\": 555 }" ;
	ASSERT_EQ(0,strcmp(expect,result))<<"fail to check query result,expect"<<expect<<" actual:"<<result ;
	// destroy bson and release cursor
	bson_destroy(&obj) ;
	bson_destroy(&cond) ;
	bson_destroy(&sel) ;
	bson_destroy(&hint) ;
	sdbReleaseCursor(cursor) ;

	// clean resource:drop cs cl disconnect releaseHandle
	cleanResource() ;
}
