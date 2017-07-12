#include <client.hpp>
#include <sdbDataSource.hpp>
#include <gtest/gtest.h>
#include <iostream>

using namespace sdbclient ;
using namespace std ;

/* 手动测试用例，用valgrind测试内存泄露
TEST( memTest, check )
{
	sdbDataSource ds ;
	sdbDataSourceConf conf ;
	string url = "localhost:11810" ;
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	sdb* conn ;
	char ch ;
	for( int i = 0;;i++ )
	{
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		ds.releaseConnection( conn ) ;
		ds.addCoord( "192.168.31.61:11910" ) ;
		ds.removeCoord( "192.168.31.61:11910" ) ;
		if( i % 10000000 == 0 )
		{
			cout << "continue??[y/n]" << endl ;
			cin >> ch ;
			if( ch == 'n' )
				break ;
		}
	}
	ASSERT_EQ( SDB_OK, ds.disable() ) ;
	ds.close() ;
}
*/
