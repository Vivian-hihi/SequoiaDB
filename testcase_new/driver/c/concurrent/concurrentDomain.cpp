/**************************************************
* @Description: test case for c driver
*               ( manual test case,not in CI )
*			    concurrent test with multi domain
* @Modify:      Liang xuewang Init
*				2016-11-10
**************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "../common/impWorker.hpp"
#include "../common/testcommon.hpp"

using import::Worker ;
using import::WorkerRoutine ;
using import::WorkerArgs ;

#define ThreadNum 5

sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbDomainHandle domain[ThreadNum] = { 0 } ;
char* domainName[ThreadNum] ;
const char* groupName = "group1" ;

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
	ASSERT_RC( rc, "fail to connect sdb in the beginning" ) ;
	// make domain name
	for( int i = 0;i < ThreadNum;i++ )
	{
	   char temp[100] = "concurrentTestDomain" ;
	   char number[10] ;
	   sprintf( number, "%d", i ) ;
	   strcat( temp, number ) ;
	   domainName[i] = strdup( temp ) ; 
	}
	// option { AutoSplit: true, Groups: [ groupName ] }
	bson option ;
   bson_init( &option ) ;
   bson_append_bool( &option, "AutoSplit", true ) ;
   bson_append_start_array( &option, "Groups" ) ;
   bson_append_string( &option, "0", groupName ) ;
   bson_append_finish_array( &option ) ;
   bson_finish( &option ) ;
   bson_print( &option ) ;
	// create domain with option
	for( int i = 0;i < ThreadNum;i++ )
	{
	   rc = sdbCreateDomain( db, domainName[i], &option, &domain[i] ) ;
	   ASSERT_RC( rc, "fail to create doamin" ) ;
	}
	bson_destroy( &option ) ;
}

void ConcurrentTest::TearDownTestCase()
{
   int rc = SDB_OK ;
   // drop domain
   for( int i = 0;i < ThreadNum;i++ )
   {
      rc = sdbDropDomain( db, domainName[i] ) ;
      ASSERT_RC( rc, "fail to drop domain" ) ;
      sdbReleaseDomain( domain[i] ) ;
      free( domainName[i] ) ;
   }
   // disconnect
   sdbDisconnect( db ) ;
   sdbReleaseConnection( db ) ;
}

class ThreadArg : public WorkerArgs
{
   public:
	  int id ;				         // rg id
} ;

void func_domain( ThreadArg* arg )
{
   int i = arg->id ;
   // printf( "i: %d\n", i ) ;
   sdbDomainHandle dom = domain[i] ;
   int rc = SDB_OK ;
   
   bson option ;
   bson_init( &option ) ;
   bson_append_bool( &option, "AutoSplit", false ) ;
   bson_finish( &option ) ;
   rc = sdbAlterDomain( dom, &option ) ;
   bson_destroy( &option ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to alter domain " << i ;
}

TEST_F( ConcurrentTest, Domain )
{
   // create multi thread to operate different domain
	Worker * workers[ThreadNum] ;
	ThreadArg arg[ThreadNum] ;
	for( int i = 0;i < ThreadNum;++i )
	{
	   arg[i].id = i ;
		workers[i] = new Worker( (WorkerRoutine)func_domain, &arg[i], false ) ;
		workers[i]->start() ;
	}
	for( int i = 0;i < ThreadNum;++i )
	{
		workers[i]->waitStop() ;
		delete workers[i] ;
	}
}
