/****************************************************
* @Description: test case for c driver
*               ( manual test case,not in CI )
*				concurrent test with multi node
* @Modify:      Liang xuewang Init
*				2016-11-10
****************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "../impWorker.hpp"
#include "../testcommon.h"

using import::Worker ;
using import::WorkerRoutine ;
using import::WorkerArgs ;

#define ThreadNum 5

sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbReplicaGroupHandle rg = SDB_INVALID_HANDLE ;
sdbNodeHandle node[ThreadNum] = { 0 } ;
const char* rgName = "mygroup" ;
const char* hostname = "sdbserver1" ;
const char* dir = "/opt/sequoiadb/database/data/" ;
char* svcName[ThreadNum] ;
char* dbPath[ThreadNum] ;

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
	rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb in the beginning" ;
	// create replicaGroup
	rc = sdbCreateReplicaGroup( db, rgName, &rg ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to create rg in the begining" ;
	
	// make svcName
	for( int i = 0;i < ThreadNum;i++)
	{
	   int number = 11900 + i*10 ;
	   char temp[10] ;
	   sprintf( temp, "%d", number ) ;
	   svcName[i] = strdup( temp ) ;
	}
	// make dbPath
	for( int i = 0;i < ThreadNum;i++)
	{
	   char temp[100] ;
	   sprintf( temp, "%s%s", dir, svcName[i] ) ;
	   dbPath[i] = strdup( temp ) ;
	}
	
	// create node and get node
	for( int i = 0;i < ThreadNum;i++)
	{
	   rc = sdbCreateNode( rg, hostname, svcName[i], dbPath[i], NULL ) ;
	   EXPECT_EQ( rc, SDB_OK ) << "fail to create node " << i ;
	   rc = sdbGetNodeByHost( rg, hostname, svcName[i], &node[i] ) ;
	   EXPECT_EQ( rc, SDB_OK ) << "fail to get node " << i ;
	}
}

void ConcurrentTest::TearDownTestCase()
{
	int i ;
	int rc = SDB_OK ;
	// connect to sdb
	// rc = sdbConnect( HOST, SERVER, USER, PASSWD, &db ) ;
   // ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb in the end" ;
   // remove rg
   rc = sdbRemoveReplicaGroup( db, rgName ) ;
   ASSERT_EQ( rc, SDB_OK ) << "fail to remove rg in the end" ;
	for( i = 0;i < ThreadNum;++i )
	{
		// release handle
		sdbReleaseNode( node[i] ) ;
		// free malloc space(strdup)
    	free( svcName[i] ) ;
    	free( dbPath[i] ) ;
	}
	// disconnect
   sdbDisconnect(db) ;
	// release handle
	sdbReleaseReplicaGroup( rg ) ;
	sdbReleaseConnection( db ) ;
}


class ThreadArg : public WorkerArgs
{
   public:
     sdbNodeHandle node ;	    // node
	  int tid ;				       // thread id
} ;

void func_node( ThreadArg* arg )
{
   sdbNodeHandle node = arg->node ;
   int i = arg->tid ;
   int rc = SDB_OK ;
   
   // get node address
   const char *host, *svc, *nodeName ;
   int nodeId ;
   rc = sdbGetNodeAddr( node, &host, &svc, &nodeName, &nodeId ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to get node addr,i = " << i ;
   EXPECT_STREQ( host, hostname ) << "fail to check host of node,i = " << i ;
   EXPECT_STREQ( svc, svcName[i] ) << "fail to check svc name of node, i = " << i ;
   printf( "%d: nodeName = %s nodeId = %d\n", i, nodeName, nodeId ) ;
   
   // start node
   rc = sdbStartNode( node ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to start node " << i ;
   // stop node
   rc = sdbStopNode( node ) ;
   EXPECT_EQ( rc, SDB_OK ) << "fail to stop node " << i ;
}

TEST_F( ConcurrentTest, Node )
{
   // create multi thread to operate different node
	Worker * workers[ThreadNum] ;
	ThreadArg arg[ThreadNum] ;
	for( int i = 0;i < ThreadNum;++i )
	{
		arg[i].node = node[i] ;
		arg[i].tid = i ; 
		workers[i] = new Worker( (WorkerRoutine)func_node, &arg[i], false ) ;
		workers[i]->start() ;
	}
	for(int i=0;i<ThreadNum;++i)
	{
		workers[i]->waitStop() ;
		delete workers[i] ;
	}
}
