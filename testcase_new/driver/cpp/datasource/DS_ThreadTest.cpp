#include <gtest/gtest.h>
#include <sdbDataSource.hpp>
#include <iostream>
#include "../impWorker.hpp"
#include "DS_thread.hpp"
#include "DS_common.hpp"

// 定义线程数量
#define ThreadNum 5 

/*
// init与init之间并发  正常获取释放连接
TEST(ThreadTest,init_init)
{
	sdbclient::sdbDataSource ds;
	DsArgs args(ds) ;
	import::Worker * workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)init, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i]->waitStop() ;
		delete workers[i] ;
	}
	EXPECT_EQ(SDB_OK,ds.enable()) ;

	sdbclient::sdb* conn = NULL ;
	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;		
	ds.releaseConnection(conn) ;	
	EXPECT_EQ(SDB_OK,ds.disable()) ;					
	ds.close() ;					
}

// init与enable之间并发，正常获取释放连接
TEST(ThreadTest,init_enable)
{
	sdbclient::sdbDataSource ds;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)init_enable, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i]->waitStop() ;
		delete workers[i] ;
	}

	sdbclient::sdb* conn = NULL ;
	if(args.getEnabled())
		EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;	
	else
		EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;	
	ds.releaseConnection(conn) ;	
	EXPECT_EQ(SDB_OK,ds.disable()) ;	
	ds.close() ;					
}

// init与disable之间并发，不出现死锁
TEST(ThreadTest,init_disable)
{
	sdbclient::sdbDataSource ds;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)init_disable, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
 	{
 		workers[i]->waitStop() ;
 		delete workers[i] ;
 	 }
	ds.close() ;					
}

// init与close之间并发，不出现死锁
TEST(ThreadTest,init_close)
{
	sdbclient::sdbDataSource ds;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)init_disable, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {   
        workers[i]->waitStop() ;
        delete workers[i] ;
     } 
}

// init与getConnection/releaseConnection之间并发，没有init时获取连接出错
TEST(ThreadTest,init_conn)
{
	sdbclient::sdbDataSource ds;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)init_conn, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}


// init与addCoord/removeCoord之间并发
TEST(ThreadTest,init_coord)
{
	sdbclient::sdbDataSource ds;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)init_coord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}
*/

// enable与enable之间并发，正常获取释放连接
TEST(ThreadTest,enable)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)enable, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	sdbclient::sdb *conn = NULL ;
	EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
	ds.close() ;
}

// enable与disable之间并发，disable时获取连接出错
TEST(ThreadTest,enable_disable)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)enable_disable, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	sdbclient::sdb *conn = NULL ;
	EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;
	ds.close() ;
}

/*
// enable与close之间并发，close时获取连接出错
TEST(ThreadTest,enable_close)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)enable_close, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	sdbclient::sdb *conn = NULL ;
	EXPECT_EQ(SDB_DS_NOT_ENABLE,ds.getConnection(conn)) ;
	ds.close() ;
}
*/

// enable与getConnection/releaseConnection之间并发，enable之前获取连接出错
TEST(ThreadTest,enable_conn)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)enable_conn, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// enable与addCoord/removeCoord之间并发,
// init之后能够添加删除节点，添加删除节点正常
TEST(ThreadTest,enable_coord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)enable_coord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}


// disable与disable之间并发，不出错不死锁
TEST(ThreadTest,disable)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)disable, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

/*
// disable与close之间并发，close后正常disable
TEST(ThreadTest,disable_close)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)disable_close, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}
*/

// disable与getConnection/releaseConnection之间并发，disable后获取连接出错
TEST(ThreadTest,disable_conn)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)disable_conn, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}


// disable与addCoord/removeCoord之间并发
TEST(ThreadTest,disable_coord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)disable_coord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

/*
// close与close之间并发，无死锁
TEST(ThreadTest,dsclose)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)dsclose, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
}
*/

/*
// close与getConnection/releaseConnection之间并发，close后获取连接出错
TEST(ThreadTest,dsclose_conn)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)dsclose_conn, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
}
*/

/*
// close与addCoord/removeCoord之间并发
TEST(ThreadTest,dsclose_coord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)dsclose_coord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
}
*/

// getConnection与getConnection/releaseConnection之间并发，正常获取释放连接
TEST(ThreadTest,connection)
{
	getConf() ;
	sdbclient::sdbDataSource ds ;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)connection, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// getConnection与addCoord/removeCoord之间并发，正常获取释放连接
TEST(ThreadTest,connection_coord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)connection_coord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// releaseConnection与releaseConnection之间并发，正常获取连接
TEST(ThreadTest,releaseConn)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	std::vector<sdbclient::sdb *> vec ;
	int cnt = 0 ;
	while(cnt < 10)
	{
		sdbclient::sdb *conn = NULL ;
		EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
		vec.push_back(conn) ;
		++cnt ;
	}
	DsArgs args(ds,vec) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)releaseConn, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// releaseConnection与addCoord/removeCoord之间并发，正常获取释放连接
TEST(ThreadTest,releaseConn_coord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	std::vector<sdbclient::sdb *> vec ;
	int cnt = 0 ;
	while(cnt < 10)
	{
		sdbclient::sdb *conn = NULL ;
		EXPECT_EQ(SDB_OK,ds.getConnection(conn)) ;
		vec.push_back(conn) ;
		++cnt ;
	}
	DsArgs args(ds,vec) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)releaseConn_coord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// addCoord与addCoord之间并发，正常添加节点
TEST(ThreadTest,addCoord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)addCoord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// addCoord与removeCoord之间并发，正常添加删除节点
TEST(ThreadTest,addCoord_remove)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)addCoord_remove, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

// removeCoord与removeCoord之间并发，正常删除节点
TEST(ThreadTest,removeCoord)
{
	getConf() ;
	sdbclient::sdbDataSource ds;
	string url = COORD ;
	sdbclient::sdbDataSourceConf conf ;
	conf.setSyncCoordInterval(0) ;
	EXPECT_EQ(SDB_OK,ds.init(url,conf)) ;
	EXPECT_EQ(SDB_OK,ds.enable()) ;
	string url2 = COORD ;
	ds.addCoord(url2) ;
	EXPECT_EQ(1,ds.getNormalCoordNum()) ;
	DsArgs args(ds) ;
	import::Worker *workers[ThreadNum] ;
	for(int i = 0;i < ThreadNum;++i)
	{
		workers[i] = new import::Worker((import::WorkerRoutine)removeCoord, &args, false) ;
		workers[i]->start() ;
	}
	for(int i = 0;i < ThreadNum;++i)
    {
        workers[i]->waitStop() ;
        delete workers[i] ;
    }
	ds.close() ;
}

