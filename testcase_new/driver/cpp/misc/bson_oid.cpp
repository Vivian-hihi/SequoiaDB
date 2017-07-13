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

const char* csName = "bsonTestCS" ;
const char* clName = "bsonTestCL" ;
sdb _db ;
sdbCollectionSpace _cs ;
sdbCollection _cl ;

int setup()
{
	int rc = SDB_OK ;
	rc = createNormalCl( _db, _cs, _cl, csName, clName ) ;
	CHECK_RC( rc, "fail to create normal cl, rc = %d\n", rc ) ;	
done:
	return rc ;
error:
	goto done ;
}

int teardown()
{
	int rc = SDB_OK ;
	rc = _db.dropCollectionSpace( csName ) ;
	CHECK_RC( rc, "fail to drop cs %s, rc = %d\n", csName, rc ) ;
	_db.disconnect() ;
done:
	return rc ;
error:
	goto done ;
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
   	int i = 0 ;
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

TEST( BsonTest, multiBulkInsert )
{
    int rc = SDB_OK ;
	rc = setup() ;
	ASSERT_EQ( rc, SDB_OK ) ;

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
   	rc = _cl.getCount( count ) ;
   	ASSERT_EQ( RecordNum * ThreadNum, count ) ;

	rc = teardown() ;
	ASSERT_EQ( rc, SDB_OK ) ;
}
