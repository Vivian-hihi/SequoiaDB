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
#include "../common/testcommon.hpp"

const char* CsModName = "testSSLCs" ;
char CsName[100] ;
const char* ClName = "testSSLCl" ;
sdbConnectionHandle connection = 0 ;
sdbCSHandle cs = 0 ;
sdbCollectionHandle cl = 0 ;

// 关闭ssl时，测试sdbConnect连接
TEST(ssl,sdbConnect)
{
	int rc = SDB_OK ;
	// 使用sdbSecureConnect连接时出错
	getConf() ;
	rc = sdbSecureConnect(HOSTNAME,SVCNAME,USER,PASSWD,&connection) ;
	ASSERT_EQ(SDB_NETWORK,rc) << "fail to test secure connect when ssl is closed" ;
	
	char ConnAddr[200] ;
	sprintf(ConnAddr,"%s:%s",HOSTNAME,SVCNAME) ;
	const char* ConnAddrs[1] ;
	ConnAddrs[0] = ConnAddr ;
	rc = sdbSecureConnect1(ConnAddrs,1,USER,PASSWD,&connection) ;
	ASSERT_EQ(SDB_NET_CANNOT_CONNECT,rc) << "fail to test secure connect when ssl is close again" ;

	// 使用sdbConnect连接时正常创建集合空间集合
	getUniqueName(CsModName,CsName) ;
	rc = sdbConnect(HOSTNAME,SVCNAME,USER,PASSWD,&connection) ;
	ASSERT_EQ(SDB_OK,rc) << "fail to test connect when ssl is closed" ;
	rc = sdbCreateCollectionSpace(connection,CsName,SDB_PAGESIZE_4K,&cs) ;
	ASSERT_EQ(SDB_OK,rc) << "fail to create cs" ;
	rc = sdbCreateCollection(cs,ClName,&cl) ;
	ASSERT_EQ(SDB_OK,rc) << "fail to create cl" ;

	// 删除集合空间集合，断开连接
	rc = sdbDropCollection(cs,ClName) ;
	ASSERT_EQ(SDB_OK,rc) << "fail to drop cl" ;
	rc = sdbDropCollectionSpace(connection,CsName) ;
	ASSERT_EQ(SDB_OK,rc) << "fail to drop cs" ;
	sdbDisconnect(connection) ;

	// 释放句柄
	sdbReleaseCollection(cl) ;
 	sdbReleaseCS(cs) ;
 	sdbReleaseConnection(connection) ;
}
