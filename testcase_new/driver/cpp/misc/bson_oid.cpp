/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1285
* @Modify:      Liang xuewang Init
*			 	2017-02-28
***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <vector>
#include "../testcommon.hpp"
#include "../impWorker.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;
using namespace import ;

const CHAR* USER   = "" ;
const CHAR* PASSWD = "" ;
const CHAR* csName = "bsonTestCS" ;
const CHAR* clName = "bsonTestCL" ;
sdb _db ;
sdbCollectionSpace _cs ;
sdbCollection _cl ;

class BsonTest : public testing::Test
{
public:
	static void SetUpTestCase() ;
	static void TearDownTestCase() ;
} ;

void BsonTest::SetUpTestCase()
{
	INT32 rc = SDB_OK ;
	// connect and create cs cl
	getConf() ;
	rc = _db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	ASSERT_RC( rc, "fail to connect sdb" )
	rc = _db.createCollectionSpace( csName, SDB_PAGESIZE_4K, _cs ) ;
	ASSERT_RC( rc, "fail to create cs" )
	rc = _cs.createCollection( clName, _cl ) ;
	ASSERT_RC( rc, "fail to create cl" )	
}

void BsonTest::TearDownTestCase()
{
	INT32 rc = SDB_OK ;
	// drop cs and disconnect
	rc = _db.dropCollectionSpace( csName ) ;
	ASSERT_RC( rc, "fail to drop cs" )
	_db.disconnect() ;
}

class ThreadArgs : public WorkerArgs
{
public:
   INT32 num ;
   INT32 tid ;
} ;

void bulkInsert( ThreadArgs* args )
{
   INT32 num = args->num ;
   INT32 tid = args->tid ;
   INT32 rc = SDB_OK ;
   sdb db ;
   sdbCollectionSpace cs ;
   sdbCollection cl ;

   // connect and get cs cl
   rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to connect in thread " << tid ; 
   rc = db.getCollectionSpace( csName, cs ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to get cs in thread " << tid ;
   rc = cs.getCollection( clName, cl ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to get cl in thread " << tid ;  

   // bulk insert record
   vector<BSONObj> vec ;
   INT32 i = 0 ;
   while( i < num )
   {
      BSONObj obj = BSON( "a" << i + tid * num ) ;
	  vec.push_back( obj ) ;
      i++ ;
   }
   rc = cl.bulkInsert( 0, vec ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to bulk insert" ; 

   // disconnect and release handle
   db.disconnect() ;
}

TEST_F( BsonTest, multiBulkInsert )
{
   INT32 rc = SDB_OK ;
   INT32 ThreadNum = 20 ;
   INT32 RecordNum = 100 ;
   Worker* workers[ThreadNum] ;
   ThreadArgs args[ThreadNum] ;
   for( INT32 i = 0;i < ThreadNum;i++ )
   {
      args[i].num = RecordNum ;
      args[i].tid = i ;
      workers[i] = new Worker( (WorkerRoutine)bulkInsert, &args[i], false ) ;
      workers[i]->start() ;
   }
   for( INT32 i = 0;i < ThreadNum;i++ )
   {
      workers[i]->waitStop() ;
   }
   SINT64 count = 0 ;
   rc = _cl.getCount( count ) ;
   ASSERT_EQ( RecordNum * ThreadNum, count ) ;
}
