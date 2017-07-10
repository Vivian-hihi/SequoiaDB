/*************************************************
* @Description: test case for c driver
*				concurrent test with multi cursor
* @Modify:      Liang xuewang Init
*				2016-11-10
***************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "../common/impWorker.hpp"
#include "../common/testcommon.hpp"

using import::Worker ;
using import::WorkerRoutine ;
using import::WorkerArgs ;

#define ThreadNum 5
#define recordNum 100

sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbCSHandle cs = SDB_INVALID_HANDLE ;
sdbCollectionHandle cl = SDB_INVALID_HANDLE ;
const char* CsModName = "concurrentTestCs" ;
char CsName[100] ;
const char* ClName = "concurrentTestCl" ;
sdbCursorHandle cursor[ThreadNum] ;

class ConcurrentTest : public testing::Test
{
	public:
	// run before all testcases
	static void SetUpTestCase() ;
	// run after all testcases
	static void TearDownTestCase() ;
} ;

void ConcurrentTest::SetUpTestCase()
{
   // connect to sdb
	int rc = SDB_OK ;
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb in the beginning" ;
	// create cs
	getUniqueName( CsModName,CsName ) ;
	rc = sdbCreateCollectionSpace( db, CsName, SDB_PAGESIZE_4K, &cs ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cs" ;
	// create cl 
	rc = sdbCreateCollection( cs, ClName, &cl ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create cl" ;
	// insert records { a: i, flag:1 }
	for( int i = 0;i < recordNum;i++ )
	{
	   bson obj ;
	   bson_init( &obj ) ;
	   bson_append_int( &obj, "a", i ) ;
	   bson_append_int( &obj, "flag", 1 ) ;
	   bson_finish( &obj ) ;
	   rc = sdbInsert( cl, &obj ) ;
	   ASSERT_EQ( rc, SDB_OK ) << "fail to insert record " << i ;
	   bson_destroy( &obj ) ; 
	}
	// query record
	bson cond ;
	bson_init( &cond ) ;
	bson_append_int( &cond, "flag", 1 ) ;
	bson_finish( &cond ) ;
	bson sel ;
	bson_init( &sel ) ;
	bson_append_string( &sel, "a", "" ) ;
	bson_finish( &sel ) ;
	for( int i = 0;i < ThreadNum;i++ )
	{
	   rc = sdbQuery( cl, &cond, &sel, NULL, NULL, 0, -1, &cursor[i] ) ;
	   ASSERT_EQ( rc, SDB_OK ) << "fail to query record " << i ;
	}
	bson_destroy( &cond ) ;
	bson_destroy( &sel ) ;
}

void ConcurrentTest::TearDownTestCase()
{
   int rc = SDB_OK ;
   // drop cs
   rc = sdbDropCollectionSpace( db, CsName ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to drop cs" ;
   // release cursor
   for( int i = 0;i < ThreadNum;i++ )
      sdbReleaseCursor( cursor[i] ) ;
   // disconnect
   sdbDisconnect( db ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

class ThreadArg : public WorkerArgs
{
   public:
     sdbCursorHandle cursor ;    // cursor handle
	  int id ;				         // cursor id
} ;

void func_cursor( ThreadArg* arg )
{
   sdbCursorHandle cursor = arg->cursor ;
   int i = arg->id ;
   int rc = SDB_OK ;
   
   bson obj ;
   bson_init( &obj ) ;
   int value = 0 ;
   while( !(rc = sdbNext(cursor,&obj)) )
   {
      bson_iterator it ;
      bson_iterator_init( &it, &obj ) ;
      ASSERT_EQ( value, bson_iterator_int( &it ) ) << "fail to check cursor " << i ;
      value++ ;
      bson_destroy( &obj ) ;
      bson_init( &obj ) ;
   }
   bson_destroy( &obj ) ;
}

TEST_F( ConcurrentTest, Cursor )
{
   // create multi thread to operate different cursor
	Worker * workers[ThreadNum] ;
	ThreadArg arg[ThreadNum] ;
	for( int i = 0;i < ThreadNum;++i )
	{
		arg[i].cursor = cursor[i] ;
		arg[i].id = i ; 
		workers[i] = new Worker((WorkerRoutine)func_cursor, &arg[i], false) ;
		workers[i]->start() ;
	}
	for( int i = 0;i < ThreadNum;++i )
	{
		workers[i]->waitStop() ;
		delete workers[i] ;
	}
}
	
