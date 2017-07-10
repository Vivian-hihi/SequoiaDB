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
#include "../common/testcommon.hpp"

using import::Worker ;
using import::WorkerRoutine ;
using import::WorkerArgs ;

#define ThreadNum 5

sdbConnectionHandle db = SDB_INVALID_HANDLE ;
sdbReplicaGroupHandle rg[ThreadNum] = { 0 } ;
char* rgName[ThreadNum] ;

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
      char temp[50] = "lxw_datagroup" ;
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
	   ASSERT_EQ( rc, SDB_OK ) << "fail to create rg " << i ;
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
		ASSERT_EQ( rc, SDB_OK ) << "fail to remove rg "<<i ;
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
   sprintf( svcName1, "%d", atoi(RSRVPORTBEGIN)+i*10 ) ;
   char svcName2[10] ;
   sprintf( svcName2, "%d", atoi(RSRVPORTBEGIN)+i*10+5 ) ;
   
   // make dbPath1 2
   char dbPath1[100], dbPath2[100] ;
   sprintf( dbPath1, "%s%s", RSRVNODEDIR, svcName1 ) ;
   sprintf( dbPath2, "%s%s", RSRVNODEDIR, svcName2 ) ;
   
   // create node1 2
   int rc = SDB_OK ;
   rc = sdbCreateNode( rg, IPADDR, svcName1, dbPath1, NULL ) ;
   if( rc != SDB_OK )
   {
      printf( "fail to create node1 in rg %d,ip: %s,svcname: %s,dbpath: %s\n", i, IPADDR, svcName1, dbPath1 ) ;
	  return ;
   }
   rc = sdbCreateNode( rg, IPADDR, svcName2, dbPath2, NULL ) ;
   if( rc != SDB_OK )
   {
      printf( "fail to create node2 in rg %d,ip: %s,svcname: %s,dbpath: %s\n", i, IPADDR, svcName2, dbPath2 ) ;
      return ;
   }	
   
   // start rg
   rc = sdbStartReplicaGroup( rg ) ;
   if( rc != SDB_OK )
   {
      printf( "fail to start rg %d\n", i ) ;
      return ;
   }
   
   // stop rg
   rc = sdbStopReplicaGroup( rg ) ;
   if( rc != SDB_OK )
   {
      printf( "fail to stop rg %d\n", i ) ;
      return ;
   }  
 
   // remove node1 success
   // cannot remove node2 which is the only one node in rg
   rc = sdbRemoveNode( rg, IPADDR, svcName1, NULL ) ;
   if( rc != SDB_OK )
   {
      printf( "fail to remove rg %d\n", i ) ;
      return ;
   }
   rc = sdbRemoveNode( rg, IPADDR, svcName2, NULL ) ;
   if( rc != SDB_CATA_RM_NODE_FORBIDDEN )
   {
	  printf( "fail to test remove node2 in rg %d\n", i ) ;
	  return ;
   }
}

TEST_F( ConcurrentTest,ReplicaGroup )
{
	getConf() ;
	getLocalIpAddr() ;
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
