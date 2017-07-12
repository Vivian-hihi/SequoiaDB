/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1285
* @Modify:      Liang xuewang Init
*			 	2017-02-28
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include "../common/testcommon.hpp"
#include "../common/impWorker.hpp"

using namespace import ;

const char* csModName = "bsonTestCS" ;
char csName[100] ;
const char* clName = "bsonTestCL" ;
sdbConnectionHandle _db = SDB_INVALID_HANDLE ;
sdbCSHandle         _cs = SDB_INVALID_HANDLE ;
sdbCollectionHandle _cl = SDB_INVALID_HANDLE ;

class BsonTest : public testing::Test
{
public:
	static void SetUpTestCase() ;
	static void TearDownTestCase() ;
} ;

void BsonTest::SetUpTestCase()
{
	int rc = SDB_OK ;
   	// connect sdb
   	getConf() ;
   	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &_db ) ;
   	ASSERT_RC( rc, "fail to connect sdb, rc = %d\n", rc ) ;
   	// create cs cl
   	getUniqueName( csModName, csName ) ;
   	rc = sdbCreateCollectionSpace( _db, csName, SDB_PAGESIZE_4K, &_cs ) ;
   	ASSERT_RC( rc, "fail to create cs %s, rc = %d\n", csName, rc ) ;
   	rc = sdbCreateCollection( _cs, clName, &_cl ) ;
   	ASSERT_RC( rc, "fail to create cl %s, rc = %d\n", clName, rc ) ;     
}

void BsonTest::TearDownTestCase()
{
   	int rc = SDB_OK ;
   	// drop cs
   	rc = sdbDropCollectionSpace( _db, csName ) ;
   	ASSERT_RC( rc, "fail to drop cs %s, rc = %d\n", csName, rc ) ;
   	// disconnect and release handle
   	sdbDisconnect( _db ) ;
   	sdbReleaseCollection( _cl ) ;
   	sdbReleaseCS( _cs ) ;
   	sdbReleaseConnection( _db ) ;
}

class ThreadArgs : public WorkerArgs
{
public:
	int num ;
   	int tid ;
} ;

void bulkInsert( ThreadArgs* args )
{
   	int num = args->num ;
   	int tid = args->tid ;
   	int rc = SDB_OK ;
   	sdbConnectionHandle db ;
   	sdbCSHandle cs ;
   	sdbCollectionHandle cl ;

   	// connect and get cs cl
   	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
   	ASSERT_RC( rc, "fail to connect in thread %d, rc = %d\n", tid, rc ) ;
   	rc = sdbGetCollectionSpace( db, csName, &cs ) ;
   	ASSERT_RC( rc, "fail to get cs %s in thread %d, rc = %d\n", csName, tid, rc ) ;
   	rc = sdbGetCollection1( cs, clName, &cl ) ;
   	ASSERT_RC( rc, "fail to get cl %s in thread %d, rc = %d\n", clName, tid, rc ) ; 

   	// bulk insert record
   	int i = 0 ;
   	bson* rec[num] ;
   	while( i < num )
   	{
    	rec[i] = bson_create() ;
      	bson_append_int( rec[i], "a", i + tid * num ) ;
      	bson_finish( rec[i] ) ;
      	i++ ;
   	}
   	rc = sdbBulkInsert( cl, 0, rec, num ) ;
   	ASSERT_RC( rc, "fail to bulk insert in thread %d, rc = %d\n", tid, rc ) ;
   	i = 0 ;
   	while( i < num )
   	{
    	bson_dispose( rec[i] ) ;
	  	i++ ;
   	}

   	// disconnect and release handle
   	sdbDisconnect( db ) ;
   	sdbReleaseCollection( cl ) ;
   	sdbReleaseCS( cs ) ;
   	sdbReleaseConnection( db ) ;
}

TEST_F( BsonTest, multiBulkInsert )
{
	int rc = SDB_OK ;
   	int ThreadNum = 20 ;
   	int RecordNum = 100 ;
   	Worker* workers[ThreadNum] ;
   	ThreadArgs args[ThreadNum] ;
   	for( int i = 0;i < ThreadNum;i++ )
   	{
    	args[i].num = RecordNum ;
      	args[i].tid = i ;
      	workers[i] = new Worker( (WorkerRoutine)bulkInsert, &args[i], false ) ;
      	workers[i]->start() ;
   	}
   	for( int i = 0;i < ThreadNum;i++ )
   	{
      	workers[i]->waitStop() ;
   	}
   	SINT64 count = 0 ;
   	rc = sdbGetCount( _cl, NULL, &count ) ;
   	ASSERT_EQ( RecordNum * ThreadNum, count ) ;
}
