#include <sdbDataSource.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include "DS_common.hpp"

using namespace std ;
using namespace sdbclient ;

// 用户信息非法时，init/enable正常返回, getConnection报错
TEST( InvalidArgTest, userInfo )
{
	getConf() ;
    sdb db ;
	db.connect( HOSTNAME,  SVCNAME ) ;
	if( isStandalone( db ) )
	{
		cout << "Standalone can't create user" << endl ;
		return ;
	}
	sdbDataSource ds ;
	sdbDataSourceConf conf ;
	string url = COORD ;
	sdb* conn = NULL ;

	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
	ASSERT_TRUE( NULL != conn ) ;
	ASSERT_EQ( SDB_OK, conn->createUsr( "root", "sequoiadb" ) ) ;		// 首先创建一个合法用户，打开用户信息校验功能
	ds.close() ;
	
	conf.setUserInfo( "lxw", "" ) ;				// 非法用户名
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( SDB_DS_NO_REACHABLE_COORD, ds.getConnection( conn ) ) ;
	ds.close() ;
	
	conf.setUserInfo( "root", "" ) ;			    // 合法用户名无密码
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( SDB_DS_NO_REACHABLE_COORD, ds.getConnection( conn ) ) ;
	ds.close() ;
		
	conf.setUserInfo( "root", "seq" ) ;			// 合法用户名非法密码
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( SDB_DS_NO_REACHABLE_COORD, ds.getConnection( conn ) ) ;
	ds.close() ;

	conf.setUserInfo( "root", "sequoiadb" ) ;		// 最后删除用户，防止后续连接被检验用户信息
	ASSERT_EQ( SDB_OK, ds.init( url, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
	ASSERT_EQ( SDB_OK, conn->removeUsr( "root", "sequoiadb" ) ) ;
	ds.releaseConnection( conn ) ;
	ds.close() ;
}
