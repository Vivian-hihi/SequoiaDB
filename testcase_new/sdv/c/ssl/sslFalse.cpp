/***************************************************************
* @Description: test case for Jira questionaire
*				( manual test case,not in CI )
*				SEQUOIADBMAINSTREAM-1958
*				test ssl with ssl is forbidden in configure file
* @Modify:		Liang xuewang Init
*				2016-11-10
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char HostName[] = "192.168.31.61" ;
char SvcName[] = "11810" ;
const char *ConnAddr = "192.168.31.61:11810" ;
char UsrName[] = "" ;
char Passwd[] = "" ;
char CsName[] = "c_driver_test" ;
char ClName[] = "ssl" ;
sdbConnectionHandle connection = 0 ;
sdbCSHandle cs = 0 ;
sdbCollectionHandle cl = 0 ;

// 关闭ssl时，测试sdbConnect连接
TEST(ssl,sdbConnect)
{
	// 使用sdbSecureConnect连接时出错
	EXPECT_EQ(SDB_NETWORK,sdbSecureConnect(HostName,SvcName,UsrName,Passwd,&connection)) ;
	EXPECT_EQ(SDB_NET_CANNOT_CONNECT,sdbSecureConnect1(&ConnAddr,1,UsrName,Passwd,&connection)) ;
	// 使用sdbConnect连接时正常创建集合空间集合
	ASSERT_EQ(SDB_OK,sdbConnect(HostName,SvcName,UsrName,Passwd,&connection)) ;
	ASSERT_EQ(SDB_OK,sdbCreateCollectionSpace(connection,CsName,SDB_PAGESIZE_4K,&cs)) ;
	ASSERT_EQ(SDB_OK,sdbCreateCollection(cs,ClName,&cl)) ;

	// 删除集合空间集合，断开连接
	ASSERT_EQ(SDB_OK,sdbDropCollection(cs,ClName)) ;
	ASSERT_EQ(SDB_OK,sdbDropCollectionSpace(connection,CsName)) ;
	sdbDisconnect(connection) ;
	// 释放句柄
	sdbReleaseCollection(cl) ;
 	sdbReleaseCS(cs) ;
 	sdbReleaseConnection(connection) ;
}
