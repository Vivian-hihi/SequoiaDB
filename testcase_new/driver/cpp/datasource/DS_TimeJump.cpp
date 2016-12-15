#include <sdbDataSource.hpp>
#include <gtest/gtest.h>
#include <client.hpp>

using namespace sdbclient ;

/* 手动测试用例，时间跳变测试连接超时
TEST(timeTest,jump)
{
	sdbDataSourceConf conf ;
	conf.setCheckIntervalInfo(3,6) ;
	string url = "localhost:11810" ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;

	sdb* conn = NULL ;
	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
	char flags ;
	while(true)
	{
		cout<<"wait signal,please input(y/n):" ;
		cin>>flags ;
		if(flags == 'n')
			break ;
	}
	ds.releaseConnection(conn) ;
	EXPECT_EQ(0,ds.getIdleConnNum()) ;
	ds.close() ;
}
*/
