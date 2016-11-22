/*************************************************
* @Description: test case for c driver
*				( manual test case,not in CI )
*				concurrent test with multi rg
* @Modify:      Liang xuewang Init
*				2016-11-10
*************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "../common/impWorker.hpp"
#include "../common/testcommon.h"

using import::Worker ;
using import::WorkerRoutine ;
using import::WorkerArgs ;

#define ThreadNum 5

sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbReplicaGroupHandle rg[ThreadNum] = { 0 } ;
char* rgName[ThreadNum] ;
const char* hostname = "sdbserver1" ;
const char* dir = "/opt/sequoiadb/database/data/" ;

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
	int i ;
	// make rgName
   for( i = 0;i < ThreadNum;++i )
   {
      char temp[20] = "datagroup" ;
      char number[10] ;
      sprintf( number, "%d", i ) ;
      strcat( temp, number ) ;
      rgName[i] = strdup( temp ) ;
   }
   // connect to sdb
	int rc = SDB_OK ;
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb in the beginning" ;
	// create replicaGroup
	for( i = 0;i < ThreadNum;++i )
	{
	   rc = sdbCreateReplicaGroup( db, rgName[i], &rg[i] ) ;
	   EXPECT_EQ( rc, SDB_OK ) << "fail to create rg " << i ;
	}
}

void ConcurrentTest::TearDownTestCase()
{
	int i ;
	int rc = SDB_OK ;
	// connect to sdb
	// rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   // ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb in the end" ;
	for( i = 0;i < ThreadNum;++i )
	{
		// remove rg
		rc = sdbRemoveReplicaGroup( db, rgName[i] ) ;
		EXPECT_EQ( rc, SDB_OK ) << "fail to remove rg "<<i ;
		// release handle
		sdbReleaseReplicaGroup( rg[i] ) ;
		// free malloc space(strdup)
    	free( rgName[i] ) ;
	}
	// disconnect
   sdbDisconnect( db ) ;
	// release handle
	sdbReleaseConnection( db ) ;
}

class ThreadArg : public WorkerArgs
{
   public:
     sdbReplicaGroupHandle rg ;	// replicaGroup
	  int rid ;				         // rg id
} ;

void func_rg( ThreadArg* arg )
{
   sdbReplicaGroupHandle rg = arg->rg ;
   int i = arg->rid ;
   
   // make svcName1 2
   char svcName1[10] ;
   sprintf( svcName1, "%d", 11900+i*100 ) ;
   char svcName2[10] ;
   sprintf( svcName2, "%d", 11900+i*100+10 ) ;
   
   // make dbPath1 2
   char dbPath1[100], dbPath2[100] ;
   sprintf( dbPath1, "%s%s", dir, svcName1 ) ;
   sprintf( dbPath2, "%s%s", dir, svcName2 ) ;
   
   // create node1 2
   int rc = SDB_OK ;
   rc = sdbCreateNode( rg, hostname, svcName1, dbPath1, NULL ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to create node1 in rg " << i ;
   rc = sdbCreateNode( rg, hostname, svcName2, dbPath2, NULL ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to create node2 in rg " << i ;
   
   // start rg
   rc = sdbStartReplicaGroup( rg ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to start rg " << i ;
   
   // stop rg
   rc = sdbStopReplicaGroup( rg ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to stop rg " << i ;
   
   // remove node1 success
   // cannot remove node2 which is the only one node in rg
   rc = sdbRemoveNode( rg, hostname, svcName1, NULL ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to remove node1 in rg " << i ;  
   rc = sdbRemoveNode( rg, hostname, svcName2, NULL ) ;
   EXPECT_EQ( rc, SDB_CATA_RM_NODE_FORBIDDEN ) << "fail to test remove node2 in rg " << i ;
}

TEST_F( ConcurrentTest,ReplicaGroup )
{
   // create multi thread to operate different rg
	Worker * workers[ThreadNum] ;
	ThreadArg arg[ThreadNum] ;
	for( int i = 0;i < ThreadNum;++i )
	{
		arg[i].rg = rg[i] ;
		arg[i].rid = i ; 
		workers[i] = new Worker( (WorkerRoutine)func_rg, &arg[i], false ) ;
		workers[i]->start() ;
	}
	for(int i=0;i<ThreadNum;++i)
	{
		workers[i]->waitStop() ;
		delete workers[i] ;
	}
}
