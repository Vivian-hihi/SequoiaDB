/**************************************************************
* @Description: test case for Jira questionaire
*				( manual test case,not in CI )
*				SEQUOIADBMAINSTREAM-1958
*				test ssl with ssl is allowed in configure file
* @Modify:      Liang xuewang Init
*               2016-11-10
**************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char HostName[] = "localhost" ;
char SvcName[] = "11810" ;
const char * ConnAddr = "localhost:11810" ;
char UsrName[] = "" ;
char Passwd[] = "" ;
char CsName[] = "c_driver_test" ;
char ClName[] = "ssl" ;
sdbConnectionHandle connection = 0 ;
sdbCSHandle cs = 0 ;
sdbCollectionHandle cl = 0 ;

// 测试开启ssl，使用sdbSecureConnect正常连接创建集合空间、集合
TEST(ssl,sdbSecureConnect)
{
	// 使用ssl连接sdb并创建集合空间、集合
	ASSERT_EQ(SDB_OK,sdbSecureConnect(HostName,SvcName,UsrName,Passwd,&connection)) ;
	ASSERT_EQ(SDB_OK,sdbCreateCollectionSpace(connection,CsName,SDB_PAGESIZE_4K,&cs)) ;
	ASSERT_EQ(SDB_OK,sdbCreateCollection(cs,ClName,&cl)) ;

	// 删除集合、集合空间、断开连接
	ASSERT_EQ(SDB_OK,sdbDropCollection(cs,ClName)) ;
	ASSERT_EQ(SDB_OK,sdbDropCollectionSpace(connection,CsName)) ;
	sdbDisconnect(connection) ;
	// 释放句柄
	sdbReleaseCollection(cl) ;
	sdbReleaseCS(cs) ;
	sdbReleaseConnection(connection) ;
}

// 测试开启ssl，使用sdbSecureConnect1正常连接创建集合空间、集合
TEST(ssl,sdbSecureConnect1)
{
	// 使用ssl连接sdb并创建集合空间、集合
	ASSERT_EQ(SDB_OK,sdbSecureConnect1(&ConnAddr,sizeof(*ConnAddr),UsrName,Passwd,&connection)) ;
	ASSERT_EQ(SDB_OK,sdbCreateCollectionSpace(connection,CsName,SDB_PAGESIZE_4K,&cs)) ;
	ASSERT_EQ(SDB_OK,sdbCreateCollection(cs,ClName,&cl)) ;

	// 删除集合、集合空间、断开连接
	ASSERT_EQ(SDB_OK,sdbDropCollection(cs,ClName)) ;
	ASSERT_EQ(SDB_OK,sdbDropCollectionSpace(connection,CsName)) ;
	sdbDisconnect(connection) ;
	// 释放句柄
	sdbReleaseCollection(cl) ;
    sdbReleaseCS(cs) ;
 	sdbReleaseConnection(connection) ;
} 
