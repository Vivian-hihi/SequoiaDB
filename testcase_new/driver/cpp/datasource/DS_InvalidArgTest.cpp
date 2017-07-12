#include <sdbDataSource.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <gtest/gtest.h>
#include "DS_common.hpp"

using namespace std ;
using namespace sdbclient ;

string url_right = COORD ;

TEST( InvalidArgTest, connCntInfo )
{
	getConf() ;
	sdbDataSource ds ;

	sdbDataSourceConf conf; 
	conf.setConnCntInfo( -3, 10, 20, 500 ) ;	// _initConnCount < 0 不合法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close(); 

	conf.setConnCntInfo( 0, 10, 20, 500 ) ;	// _initConnCount = 0 合法
	ASSERT_EQ( SDB_OK, ds.init( COORD, conf ) ) ;
	ds.close() ;

	conf.setConnCntInfo( 25, 10, 20, 500 ) ;	// _initConnCount > maxIdleCount 内部修正为maxIdleCount
	ASSERT_EQ( SDB_OK, ds.init( COORD, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	ASSERT_EQ( ds.getIdleConnNum(), conf.getMaxIdleCount() ) ;
	ds.close() ;

	conf.setConnCntInfo( 10, -3, 20, 500 ) ;    // _deltaIncCount < 0 不合法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close() ;

	conf.setConnCntInfo( 10, 0, 20, 500 ) ;       // _deltaIncCount = 0 不合法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close() ;

	conf.setConnCntInfo( 10, 25, 20, 500 ) ;      // _deltaIncCount > maxIdleCount 内部修正为maxIdleCount
	ASSERT_EQ( SDB_OK, ds.init( COORD, conf ) ) ;
	ASSERT_EQ( SDB_OK, ds.enable() ) ;
	vector<sdb*> vec ;
	while( ds.getIdleConnNum() > SDB_DS_TOPRECREATE_THRESHOLD )
	{
		sdb* conn = NULL ;
		ASSERT_EQ( SDB_OK, ds.getConnection( conn ) ) ;
		vec.push_back( conn ) ;
		// cout << ds.getIdleConnNum() << endl ;
	}
	ossSleep( 2000 ) ;
	// cout << ds.getIdleConnNum() << endl ;
	ASSERT_EQ( ds.getIdleConnNum()-SDB_DS_TOPRECREATE_THRESHOLD, conf.getMaxIdleCount() ) ;
	ds.close() ;

	conf.setConnCntInfo( 10, 10, -3, 500 ) ;		// _maxIdleCount < 0 不合法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close() ;
	conf.setConnCntInfo( 10, 10, 0, 500 ) ;		// _maxIdleCount = 0 不合法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close();
	conf.setConnCntInfo( 10, 10, 500, 500 ) ;	// _maxIdleCount = maxCount 合法
	ASSERT_EQ( SDB_OK, ds.init( COORD, conf ) ) ;
	ds.close() ;
	conf.setConnCntInfo( 10, 10, 600, 500 ) ;	// _maxIdleCount > maxCount 不合法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close() ;

	conf.setConnCntInfo( 10, 10, 20, -3 ) ;				// _maxCount < 0 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close() ;
	conf.setConnCntInfo( 10, 10, 20, 0 ) ;		         // _maxCount = 0 
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	ds.close() ;
	conf.setConnCntInfo( 10, 10, 20, 2147483647 ) ;		 // _maxCount = 边界值
	ASSERT_EQ( SDB_OK, ds.init( COORD, conf ) ) ;
	ds.close() ;
}

TEST( InvalidArgTest, checkIntervalInfo )
{
	getConf() ;
	sdbDataSource ds ;

	sdbDataSourceConf conf ;
	conf.setCheckIntervalInfo( -3, 0 ) ;		// _checkInterval < 0 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	conf.setCheckIntervalInfo( 0, 0 ) ;		// _checkInterval = 0 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
	conf.setCheckIntervalInfo( 60, 30 ) ;		// _checkInterval > keepAliveTimeoue 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;

	conf.setCheckIntervalInfo( 30, -3 ) ;		// _keepAliveTimeout < 0 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
}

TEST( InvalidArgTest, coordInterval )
{
	getConf() ;
	sdbDataSource ds ;

	sdbDataSourceConf conf ;
	conf.setSyncCoordInterval( -3 ) ;			 // _syncCoordInterval < 0 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
}

TEST( InvalidArgTest, connectStrategy )
{
	getConf() ;
	sdbDataSource ds ;

	sdbDataSourceConf conf ;
	conf.setConnectStrategy( DATASOURCE_STRATEGY( 5 ) ) ;  // _connectStrategy 不为0-3 非法
	ASSERT_EQ( SDB_INVALIDARG, ds.init( COORD, conf ) ) ;
}

//  url格式检验，检验空地址和格式不符合xxxx:xxxx的地址，init时不会报错但getConnection时会报错
TEST( InvalidArgTest, url )
{
	getConf() ;
	sdbDataSource ds ;
	sdbDataSourceConf conf;

	string url_wrong1 = "something" ;
	string url_wrong2 = "something::00000" ;
	ASSERT_EQ( SDB_INVALIDARG, ds.init( url_wrong1, conf ) ) ;
	ds.close() ;
	ASSERT_EQ( SDB_INVALIDARG, ds.init( url_wrong2, conf ) ) ;
	ds.close() ;

	string url_right1 = "something:" ;
	string url_right2 = ":000000" ;
	string url_right3 = ":" ;
	ASSERT_EQ( SDB_OK, ds.init( url_right1, conf ) ) ;
	ds.close() ;
	ASSERT_EQ( SDB_OK, ds.init( url_right2, conf ) ) ;
	ds.close() ;
	ASSERT_EQ( SDB_OK, ds.init( url_right3, conf ) ) ;
	ds.close() ;
}

// url列表测试
TEST( InvalidArgTest, urlist )
{
	sdbDataSource ds ;
	sdbDataSourceConf conf ;	
	vector<string> urlArray1( 10, "" ) ;
	vector<string> urlArray2 ;
	ASSERT_EQ( SDB_INVALIDARG, ds.init( urlArray1, conf ) ) ;
	ASSERT_EQ( SDB_INVALIDARG, ds.init( urlArray2, conf ) ) ;
}

