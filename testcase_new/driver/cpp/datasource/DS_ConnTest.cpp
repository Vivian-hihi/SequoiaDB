#include <gtest/gtest.h>
#include <sdbDataSource.hpp>
#include "DS_common.hpp"

using namespace sdbclient ;

// 启用连接池获取及释放连接，添加及删除节点
TEST(ConnTest,enableConn)
{
	getConf() ;
	sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;

	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
	ds.releaseConnection(conn) ;	// 释放连接无返回值

	ds.addCoord(COORD) ;	//
	EXPECT_EQ(1,ds.getNormalCoordNum()) ;
	ds.addCoord("localhost:11910") ;
	EXPECT_EQ(2,ds.getNormalCoordNum()) ;
	ds.addCoord("1.2.3.4:000000") ;
	EXPECT_EQ(3,ds.getNormalCoordNum()) ;
	ds.addCoord("something:000000") ;
	EXPECT_EQ(3,ds.getNormalCoordNum()) ;
	ds.removeCoord("something:000000") ;
	EXPECT_EQ(3,ds.getNormalCoordNum()) ;
	ds.removeCoord("1.2.3.4:000000") ;
	EXPECT_EQ(2,ds.getNormalCoordNum()) ;

	ds.close() ;   // 关闭连接池无返回值
}

// 启用连接池获取连接到连接池满后继续申请连接
TEST(ConnTest,fullConn)
{
	getConf() ;
	sdbDataSourceConf conf ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;

	sdb* conn = NULL ;
	vector<sdb*> vec ;
	while(vec.size() < conf.getMaxCount())
	{
		EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
		vec.push_back(conn) ;
	}
	EXPECT_EQ(SDB_DRIVER_DS_RUNOUT,ds.getConnection(conn)) ;
	ds.close() ;
}

// 禁用连接池获取连接
TEST(ConnTest,disableConn)
{
	getConf() ;
	sdbDataSourceConf conf ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_OK,ds.disable()) ;

	sdb* conn = NULL ;
	EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;
	ds.addCoord(COORD) ;			// 添加节点无返回值
	ds.removeCoord(COORD) ;		// 删除节点无返回值
	ds.close() ;
}

// 禁用连接池后资源回收情况,禁用连接池后，连接队列被清空
TEST(ConnTest,disableResource)
{
	getConf() ;
	sdbDataSourceConf conf ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	sdb* conn ;
	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
	EXPECT_EQ(9,ds.getIdleConnNum()) ;
	EXPECT_EQ(1,ds.getUsedConnNum()) ;
	EXPECT_EQ(SDB_OK,ds.disable()) ;
    EXPECT_EQ(0,ds.getIdleConnNum()) ;
	EXPECT_EQ(0,ds.getUsedConnNum()) ;
	ds.close() ;
}

// 重复禁用连接池后资源回收情况,禁用连接池后，连接队列被清空
TEST(ConnTest,disableResourceAgain)
{
	getConf() ;
	sdbDataSourceConf conf ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	sdb* conn ;
	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
	EXPECT_EQ(9,ds.getIdleConnNum()) ;
	EXPECT_EQ(1,ds.getUsedConnNum()) ;
	EXPECT_EQ(SDB_OK,ds.disable()) ;
        EXPECT_EQ(0,ds.getIdleConnNum()) ;
	EXPECT_EQ(0,ds.getUsedConnNum()) ;
	EXPECT_EQ(SDB_OK,ds.disable()) ;
        EXPECT_EQ(0,ds.getIdleConnNum()) ;
	EXPECT_EQ(0,ds.getUsedConnNum()) ;
	ds.close() ;
}

// 调用close后继续执行相关操作
TEST(ConnTest,close)
{
	getConf() ;
	sdbDataSourceConf conf ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	ds.close() ;
	
	sdb* conn ;
	EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;	
	ds.addCoord(COORD) ;  
	EXPECT_EQ(SDB_DS_NOT_INIT,ds.enable()) ; 
	EXPECT_EQ(SDB_OK,ds.disable()) ; //  close后能正常调用disable
	EXPECT_EQ(SDB_DS_NOT_INIT,ds.enable()) ; 
}


// 没有调用init就执行相关操作
TEST(ConnTest,withoutInit)
{
	getConf() ;
	sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(false) ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
        EXPECT_EQ(SDB_DS_NOT_INIT,ds.enable()) ;	
   
	EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;	
	ds.addCoord(COORD) ;	
	EXPECT_EQ(SDB_OK,ds.disable()) ;		
	ds.close() ;		
}


 //获取连接后没有释放，直接disable 正常返回
TEST(ConnTest,disableWithoutRelease)
{
	getConf() ;
	sdbDataSourceConf conf ;
	sdbDataSource ds ;
	EXPECT_EQ(SDB_OK,ds.init(COORD,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	sdb* conn = NULL ;
	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;

	EXPECT_EQ(SDB_OK,ds.disable()) ; 
	ds.close() ;
}

