/************************************************************
* @Description: test case for Jira questionaire
*               ( manual test case,not in CI ) 
*				SEQUOIADBMAINSTREAM-809
* @Modify:      Liang xuewang Init
*				2016-11-11
*************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/testcommon.hpp"

sdbConnectionHandle db = 0 ;
sdbReplicaGroupHandle dataRG = 0,tempRG = 0 ;
sdbNodeHandle tempNode = 0 ;

bool getDataRG(sdbConnectionHandle db)
{
	char dataGroupName[20] = "" ;
	bool found = false ;
	sdbCursorHandle cursor = 0 ;
	int rc = sdbListReplicaGroups(db,&cursor) ;
	if(rc != SDB_OK)
	{
		printf("fail to list Replica Groups,rc=%d\n",rc) ;
		return false ;
	}
	bson obj ;
	bson_init(&obj) ;
	while(!sdbNext(cursor,&obj))
	{
		bson_iterator it ;
		bson_iterator_init(&it,&obj) ;
		while(BSON_EOO != bson_iterator_next(&it))
		{
			const char *key = bson_iterator_key(&it) ;
			if(!strcmp(key,"GroupName"))
			{
				const char *value = bson_iterator_string(&it) ;
				// printf("%s:%s\n",key,value) ;
				if(strcmp(value,"SYSCatalogGroup") && strcmp(value,"SYSCoord"))
				{
					strcpy(dataGroupName,value) ;
					found = true ;
					break ;
				}
			}
		}
		bson_destroy(&obj) ;
		bson_init(&obj) ;
		if(found)
			break ;
	}
	bson_destroy(&obj) ;
	if(!strcmp(dataGroupName,""))
	{
		printf("fail to get data group name.\n") ;
		return false ;
	}
	// printf("dataGroupName:%s\n",dataGroupName) ;
	rc = sdbGetReplicaGroup(db,dataGroupName,&dataRG) ;
	if(rc != SDB_OK)
	{
		printf("fail to get data group.\n") ;
		return false ;
	}	
	return true ;
}

class AttachAndDetachNodeTest : public testing::Test
{
	public:
	// run before all testcases
	static void SetUpTestCase() ;
	// run after all testcases
	static void TearDownTestCase() ;
};

void AttachAndDetachNodeTest::SetUpTestCase()
{
    int rc = SDB_OK ;
    // connect to sdb
    getConf() ;
    rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
    ASSERT_RC(rc,"fail to connect sdb") ;	
}

void AttachAndDetachNodeTest::TearDownTestCase()
{
	// disconnect and release handle
    sdbDisconnect(db) ;
    sdbReleaseNode(tempNode) ;
    sdbReleaseReplicaGroup(tempRG) ;
    sdbReleaseReplicaGroup(dataRG) ;
    sdbReleaseConnection(db) ;
}

TEST_F(AttachAndDetachNodeTest,onlyAttachAndOnlyDetach)
{
	// check standalone
	if(isStandalone(db))
	{
		printf( "sdb connection is standalone.\n" ) ;
		sdbDisconnect(db) ;
		sdbReleaseConnection(db) ;
		return ;
	}

	// get data group dataRG
	ASSERT_TRUE(getDataRG(db)) ;

	// create tempNode
	getConf() ;
	int a = (atoi(RSRVPORTBEGIN) + atoi(RSRVPORTEND)) / 2 ;
	char tempNodeSvcName[10] ;
	sprintf(tempNodeSvcName,"%d",a) ;
	printf("temp node svcname: %s\n",tempNodeSvcName) ;
	
	char tempNodeDbPath[100] ;
	strcpy(tempNodeDbPath,RSRVNODEDIR) ;
	strcat(tempNodeDbPath,tempNodeSvcName) ;
	printf("temp node dbpath: %s\n",tempNodeDbPath) ;
    
	getHost() ;
	int rc = sdbCreateNode(dataRG,HOST,tempNodeSvcName,tempNodeDbPath,NULL) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to create tempNode" ;
	
	// detach tempNode from dataRG
	rc = sdbDetachNode(dataRG,HOST,tempNodeSvcName,NULL) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to detach tempNode" ;
	rc = sdbGetNodeByHost(dataRG,HOST,tempNodeSvcName,&tempNode);
	ASSERT_EQ(rc,SDB_CLS_NODE_NOT_EXIST)<<"fail to check detach" ;
	
	// create tempRG
    rc = sdbCreateReplicaGroup(db,"temp",&tempRG) ;
    ASSERT_EQ(rc,SDB_OK)<<"fail to create tempRG" ;
	
	// attach tempNode to tempRG
    rc = sdbAttachNode(tempRG,HOST,tempNodeSvcName,NULL) ;
    ASSERT_EQ(rc,SDB_OK)<<"fail to attach tempNode" ;
    rc = sdbGetNodeByHost(tempRG,HOST,tempNodeSvcName,&tempNode) ;
    ASSERT_EQ(rc,SDB_OK)<<"fail to check attach" ;
    
	// start tempRG
    rc = sdbStartReplicaGroup(tempRG) ;
    ASSERT_EQ(rc,SDB_OK)<<"fail to start tempRG" ;
	
	// stop tempRG
	rc = sdbStopReplicaGroup(tempRG) ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to stop tempRG" ;
	
	// remove tempRG
	rc = sdbRemoveReplicaGroup(db,"temp") ;
	ASSERT_EQ(rc,SDB_OK)<<"fail to remove tempRG" ;
}
