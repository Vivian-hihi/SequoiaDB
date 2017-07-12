#include <gtest/gtest.h>
#include <sdbDataSource.hpp>
#include <client.hpp>
#include <iostream>
#include <vector>
using namespace std ;
using namespace sdbclient ;

/*
// 测试单个节点，所有节点正常停止，异常停止，主机异常（手动）
TEST( coordTest, stop )
{
	string url1 = "192.168.20.165:11810" ;
	//string url2 = "192.168.20.166:11810" ;
	//string url3 = "192.168.20.166:50000" ;
	vector<string> urllist ;
	urllist.push_back( url1 ) ;
	//urllist.push_back( url2 ) ;
	//urllist.push_back( url3 ) ;

	sdbDataSourceConf conf ;
	//conf.setSyncCoordInterval( 10 * 1000 ) ;
	conf.setSyncCoordInterval( 0 ) ;
	sdbDataSource ds ;
	ASSERT_EQ( SDB_OK, ds.init( urllist, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	cout << endl ;
	
	char flags ;
	for( int i = 0;;i++ )
	{
		sdb* conn = NULL ;
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		if( i % 10 == 0 && i != 0 )
		{
			cout << "continue??[y/n]: " ;
			cin >> flags ;
			if( flags == 'n' )
				break ;
		}
	}
	ds.close() ;
}
*/
