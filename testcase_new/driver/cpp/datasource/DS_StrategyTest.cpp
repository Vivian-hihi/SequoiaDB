#include <gtest/gtest.h>
#include <sdbDataSource.hpp>
#include <iostream>
#include "DS_common.hpp"

using namespace std ;
using namespace sdbclient ;

// 手动测试用例
/*
// 同步情况下测试顺序分配策略
TEST( SerialTest, syncTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_SERIAL ) ;
	conf.setSyncCoordInterval( 1 ) ;
	string url = "192.168.31.61:11910" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ossSleep( 2*1000 ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}


// 同步情况下测试随机分配策略
TEST( RandomTest, syncTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_RANDOM ) ;
	conf.setSyncCoordInterval( 1 ) ;
	string url = "192.168.31.61:11910" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ossSleep( 2*1000 ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 同步情况下测试本地分配策略
TEST( LocalTest, syncTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_LOCAL ) ;
	conf.setSyncCoordInterval( 1 ) ;
	string url = "192.168.31.61:11910" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ossSleep( 2*1000 ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 同步情况下测试均衡分配策略
TEST( BalanceTest, syncTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_BALANCE ) ;
	conf.setSyncCoordInterval( 1 ) ;
	string url = "192.168.31.61:11910" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ossSleep( 2*1000 ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点情况下测试顺序分配策略
TEST( SerialTest, urllistTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_SERIAL ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.61:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点情况下测试随机分配策略
TEST( RandomTest, urllistTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_RANDOM ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.61:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点情况下测试本地分配策略
TEST( LocalTest, urllistTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_LOCAL ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.20:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点情况下测试均衡分配策略
TEST( BalanceTest, urllistTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_BALANCE ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.61:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化一个节点，过程中添加节点情况下测试顺序分配策略
TEST( SerialTest, addcoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_SERIAL ) ;
	conf.setSyncCoordInterval( 0 ) ;
	string url = "192.168.31.61:11810" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.addCoord( "192.168.31.61:11910" ) ;
	ds.addCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 3, ds.getNormalCoordNum() ) ;
	cout << endl ;
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化一个节点，过程中添加节点情况下测试随机分配策略
TEST( RandomTest, addcoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_RANDOM ) ;
	conf.setSyncCoordInterval( 0 ) ;
	string url = "192.168.31.61:11810" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.addCoord( "192.168.31.61:11910" ) ;
	ds.addCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 3, ds.getNormalCoordNum() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化一个节点，过程中添加节点情况下测试本地分配策略
TEST( LocalTest, addcoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_LOCAL ) ;
	conf.setSyncCoordInterval( 0 ) ;
	string url = "192.168.31.61:11810" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.addCoord( "192.168.31.61:11910" ) ;
	ds.addCoord( "192.168.31.20:11810" ) ;
	ASSERT_EQ( 3, ds.getNormalCoordNum() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化一个节点，过程中添加节点情况下测试均衡分配策略
TEST( BalanceTest, addcoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_BALANCE ) ;
	conf.setSyncCoordInterval( 0 ) ;
	string url = "192.168.31.61:11810" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.addCoord( "192.168.31.61:11910" ) ;
	ds.addCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 3, ds.getNormalCoordNum() ) ;
	
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点过程中删除节点情况下测试顺序分配策略
TEST( SerialTest, removecoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_SERIAL ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11920" ) ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.61:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.removeCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点过程中删除节点情况下测试随机分配策略
TEST( RandomTest, removecoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_RANDOM ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11920" ) ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.61:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.removeCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点过程中删除节点情况下测试本地分配策略
TEST( LocalTest, removecoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_LOCAL ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11920" ) ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.20:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.removeCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 初始化多个节点过程中删除节点情况下测试均衡分配策略
TEST( BalanceTest, removecoordTest )
{
	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DS_STY_BALANCE ) ;
	conf.setSyncCoordInterval( 0 ) ;
	vector<string> urllist ;
	urllist.push_back( "192.168.31.61:11920" ) ;
	urllist.push_back( "192.168.31.61:11910" ) ;
	urllist.push_back( "192.168.31.61:11810" ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ds.removeCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 2, ds.getNormalCoordNum() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}

// 禁用连接池再启用情况下测试均衡分配策略( 默认分配策略 )
TEST( BalanceTest, enableTest )
{
	sdbDataSourceConf conf ;
	conf.setSyncCoordInterval( 0 ) ;
	string url = "192.168.31.61:11810" ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.disable() ) ;
	ds.addCoord( "192.168.31.61:11910" ) ;
	ds.addCoord( "192.168.31.61:11920" ) ;
	ASSERT_EQ( 3, ds.getNormalCoordNum() ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	
	sdb* conn = NULL ;
	int cnt = 0 ;
	while( cnt < 10 )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		++cnt ;
	}
	ds.close() ;
}
*/
