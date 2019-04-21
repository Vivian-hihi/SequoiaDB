// TODO: need to change pNodeHostName, pNodeSvcName
#include <stdio.h>
#include <gtest/gtest.h>
#include "client.h"
#include "arguments.hpp"
#include "./common/testcommon.hpp"
#include <string>
#include <iostream>
#include <stdio.h>

using namespace std ;

sdbConnectionHandle db ;
sdbCSHandle cs ;
sdbCollectionHandle cl ;
sdbReplicaGroupHandle rg ;
sdbCursorHandle cur ;

BOOLEAN is_cluster         = FALSE ;
BOOLEAN connect_flag       = FALSE ;
BOOLEAN create_rg_flag     = FALSE ;
BOOLEAN create_node_flag   = FALSE ;
BOOLEAN create_node_flag2  = FALSE ;

//const CHAR *pHostName      = "192.168.20.166" ;
const CHAR *pHostName      = ARGS->hostName() ;
const CHAR *pSvcName       = ARGS->svcName() ;
//const CHAR *pSvcName       = "11810" ;
const CHAR *pUser          = ARGS->user() ;
const CHAR *pPassword      = ARGS->passwd() ;

const INT32 rid = 10;

const CHAR *pGroupName     = "testGroupInCpp" ;

#define NODE1_NAME (atoi( ARGS->rsrvPortBegin() ) + 2 * rid * 10)
#define NODE2_NAME (atoi( ARGS->rsrvPortBegin() ) + 2 * rid * 10 + 10)
#define tmp_buf_size 1024

CHAR tmp_buf[tmp_buf_size + 1] = { 0 } ;

/*
 * @description: test for replica group's api
 * @author: tanzhaobo
 */
class replicaGroupTest : public testing::Test
{
   public:
      replicaGroupTest() {}

   public:
      // run before all the testcase
      static void SetUpTestCase() ;

      // run before all the testcase
      static void TearDownTestCase() ;

      // run before every testcase
      virtual void SetUp() ;

      // run before every testcase
      virtual void TearDown() ;
} ;

void replicaGroupTest::SetUpTestCase()
{
   INT32 rc = SDB_OK ;
   bson option ;

   // connect
   rc = sdbConnect( pHostName, pSvcName, pUser, pPassword, &db ) ;
   if ( SDB_OK != rc )
   {
      cout << "Error: Failed to connect to database: rc = " << rc << endl ;
      return ;
   }
   else
   {
      connect_flag = TRUE ;
   }
   // check whether it's in cluster
   if ( TRUE == isCluster( db ) )
   {
      is_cluster = TRUE ;
   }
   else
   {
      return ;
   }
   
   // create rg
   rc = sdbCreateReplicaGroup( db, pGroupName, &rg ) ;
   if ( SDB_OK != rc )
   {
      sprintf( tmp_buf, "Error: Failed to create replica group[%s] rc = %d",
               pGroupName, rc ) ;
      cout << tmp_buf << endl ;
      return ;
   }
   else
   {
      create_rg_flag = TRUE ;
   }
   // create node
   bson_init( &option ) ;
   bson_append_int( &option, "logfilenum", 5 ) ;
   bson_finish( &option ) ;

   CHAR hostName[100] ;
   rc = getLocalHost( hostName, 100 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   CHAR svcName1[10] ;
   sprintf( svcName1, "%d", NODE1_NAME);

   CHAR dbPath1[100];
   sprintf( dbPath1, "%s%s%s", ARGS->rsrvNodeDir(), "data/", svcName1 ) ;

   rc = sdbCreateNode( rg, hostName, svcName1, dbPath1, &option ) ;
   bson_destroy( &option ) ;
   if ( SDB_OK != rc )
   {
      sprintf( tmp_buf, "Error: Failed to create data node[%s:%s] in replica group[%s], "
               "rc = %d", hostName, svcName1, pGroupName, rc ) ;
      cout << tmp_buf << endl ;
      return ;
   }
   // start node
   rc = sdbStartReplicaGroup( rg ) ;
   if ( SDB_OK != rc )
   {
      sprintf( tmp_buf, "Error: Failed to start data node[%s:%s] in replica group, "
               "rc = %d", hostName, svcName1, rc ) ;
      cout << tmp_buf << endl ;
      return ;
   }
   else
   {
      create_node_flag = TRUE ;
   }
} 

void replicaGroupTest::TearDownTestCase()
{
   INT32 rc = SDB_OK ;
   if ( TRUE == is_cluster )
   {
      rc = sdbRemoveReplicaGroup( db, pGroupName ) ;
      if ( SDB_OK != rc )
      {
         sprintf( tmp_buf, "Error: Failed to remove replica group[%s], rc = %d",
                  pGroupName, rc ) ;
         cout << tmp_buf << endl ;
      }
   }
   sdbDisconnect( db ) ; 
}

void replicaGroupTest::SetUp()
{
   INT32 rc = SDB_OK ;
   bson option ;
   
   // check in cluster or not
   if ( FALSE == is_cluster )
      return ;

   create_node_flag2 = FALSE ;
   // create node
   bson_init( &option ) ;
   bson_append_int( &option, "logfilenum", 5 ) ;
   bson_finish( &option ) ;

   CHAR hostName[100] ;
   rc = getLocalHost( hostName, 100 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   CHAR svcName2[10] ;
   sprintf( svcName2, "%d", NODE2_NAME) ;

   CHAR dbPath2[100] ;
   sprintf( dbPath2, "%s%s%s", ARGS->rsrvNodeDir(), "data/", svcName2 ) ;

   rc = sdbCreateNode( rg, hostName, svcName2, dbPath2, &option ) ;
   if ( SDB_OK != rc )
   {
      sprintf( tmp_buf, "Error: Failed to create data node[%s:%s] in "
               "replica group[%s], rc = %d", hostName,
               svcName2, pGroupName, rc ) ;
      cout << tmp_buf << endl ;
      return ;
   }
   // start node
   rc = sdbStartReplicaGroup( rg ) ;
   if ( SDB_OK != rc )
   {
      sprintf( tmp_buf, "Error: Failed to start data node in replica "
               "group[%s], rc = %d", pGroupName, rc ) ;
      cout << tmp_buf << endl ;
      return ;
   }
   else
   {
      create_node_flag2 = TRUE ;
   }
}

void replicaGroupTest::TearDown()
{
   INT32 rc = SDB_OK ;

   // check in cluster or not
   if ( FALSE == is_cluster )
      return ;

   CHAR hostName[100] ;
   rc = getLocalHost( hostName, 100 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   CHAR svcName2[10] ;
   sprintf( svcName2, "%d", atoi( ARGS->rsrvPortBegin() ) + 2 * rid * 10 + 10 ) ;

   rc = sdbRemoveNode( rg, hostName, svcName2, NULL ) ;
   if ( SDB_OK != rc )
   {
      sprintf( tmp_buf, "Error: Failed to remove data node[%s:%s] in replica "
               "group[%s], rc = %d", hostName, svcName2,
               pGroupName, rc ) ;
      cout << tmp_buf << endl ;
   }
   create_node_flag2 = FALSE ;
}

replicaGroupTest *rg_env ;

INT32 _tmain( INT32 argc, CHAR* argv[] )
{
   testing::InitGoogleTest( &argc, argv ) ;
   return RUN_ALL_TESTS() ;
}


TEST_F( replicaGroupTest, init_test )
{
   // check in cluster or not
   if ( FALSE == is_cluster )
      return ;
   ASSERT_EQ( TRUE, connect_flag ) << "Error: Failed to connect to database" ;
   ASSERT_EQ( TRUE, create_rg_flag ) << "Error: Failed to create rg in database" ; 
   ASSERT_EQ( TRUE, create_node_flag ) << "Error: Failed to create data node in database" ;
   ASSERT_EQ( TRUE, create_node_flag2 ) << "Error: Failed to create data node in database" ;
}

TEST_F( replicaGroupTest, getRGName )
{
   // check in cluster or not
   if ( FALSE == is_cluster )
      return ;
   ASSERT_EQ( TRUE, connect_flag ) << "Error: Failed to connect to database" ;
   ASSERT_EQ( TRUE, create_rg_flag ) << "Error: Failed to create rg in database" ;
   ASSERT_EQ( TRUE, create_node_flag ) << "Error: Failed to create data node in database" ;
   ASSERT_EQ( TRUE, create_node_flag2 ) << "Error: Failed to create data node in database" ;

   INT32 rc                 = SDB_OK ;
   CHAR pBuffer[ NAME_LEN ] = { 0 } ;
   CHAR *pBuffer2           = NULL ;

   rc = sdbGetRGName( rg, pBuffer, NAME_LEN ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( 0, strncmp( pBuffer, pGroupName, strlen(pGroupName) ) ) ;

   rc = sdbGetRGName( rg, pBuffer, 1 ) ;
   ASSERT_EQ( SDB_INVALIDSIZE, rc ) ;
 
   rc = sdbGetRGName( rg, pBuffer2, 1 ) ;
   ASSERT_EQ( SDB_INVALIDARG, rc ) ;
}

TEST_F( replicaGroupTest, detachNode )
{
   // check in cluster or not
   if ( FALSE == is_cluster )
   {
      cout << "Warning: it's not a cluster environment." << endl ;
      return ;
   }
   ASSERT_EQ( TRUE, connect_flag ) << "Error: Failed to connect to database" ;
   ASSERT_EQ( TRUE, create_rg_flag ) << "Error: Failed to create rg in database" ;
   ASSERT_EQ( TRUE, create_node_flag ) << "Error: Failed to create data node in database" ;
   ASSERT_EQ( TRUE, create_node_flag2 ) << "Error: Failed to create data node in database" ;

   INT32 rc = SDB_OK ;
   sdbNodeHandle node ;

   CHAR hostName[100] ;
   rc = getLocalHost( hostName, 100 ) ;
   ASSERT_EQ( SDB_OK, rc ) ;

   CHAR svcName2[10] ;
   sprintf( svcName2, "%d", NODE2_NAME ) ;

   //SEQUOIADBMAINSTREAM-4004
   // detach node 
   bson option;
   bson_init( &option );
   bson_append_bool( &option,"KeepData", true ) ;
   bson_finish( &option ) ;
   rc = sdbDetachNode( rg, hostName, svcName2, &option ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to detach data node from group " <<
   pGroupName << ", rc = " << rc ;

   // check
   rc = sdbGetNodeByHost( rg, hostName, svcName2, &node ) ;
   ASSERT_EQ( SDB_CLS_NODE_NOT_EXIST, rc ) << "What we expect is "
      "SDB_CLS_NODE_NOT_EXIST, but rc = " << rc ;

   // attach node
   rc = sdbAttachNode( rg, hostName, svcName2, &option ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to attach data node to group " <<
      pGroupName << ", rc = " << rc ;
  
   // check 
   rc = sdbGetNodeByHost( rg, hostName, svcName2, &node ) ;
   ASSERT_EQ( SDB_OK, rc ) << "Failed to get data node from group " <<
      pGroupName << ", rc = " << rc ;
   bson_destroy( &option ) ;
}

