#include <sdbDataSource.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include "DS_common.hpp"

using namespace std ;
using namespace sdbclient ;

// 地址格式合法但不存在时，init/enable正常返回,getConnection报错
TEST( InvalidArgTest, url )
{
	sdbDataSource ds ;
	sdbDataSourceConf conf;
	sdb* conn = NULL ;

	string url_wrong = "something:00000" ;
	ASSERT_EQ( SDB_OK, ds.init( url_wrong, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( SDB_DS_NO_REACHABLE_COORD, ds.getConnection( conn ) ) ;	
	ds.close() ;
}
