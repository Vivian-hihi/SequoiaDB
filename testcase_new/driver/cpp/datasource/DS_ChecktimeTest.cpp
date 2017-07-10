#include <gtest/gtest.h>
#include <sdbDataSource.hpp>
#include <iostream>
#include <vector>
#include "DS_common.hpp"

using namespace std ;
using namespace sdbclient ;

// 休眠时间>checkInterval,检查连接池空闲连接数量= maxIdleConnNum
TEST(TimeTest,checkIntervalLong)
{
	getConf() ;
	sdbDataSourceConf conf ;
	string url = COORD ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
	vector<sdb*> vec ;

	conf.setCheckIntervalInfo(3000,0) ;
	ASSERT_EQ(SDB_OK,ds.init(url,conf)) ;
	ASSERT_EQ(SDB_OK,ds.enable()) ;
	while(vec.size() <= conf.getMaxIdleCount())
	{
		ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;
		// printf( "vec size: %d, max idle count: %d\n", vec.size(), conf.getMaxIdleCount() ) ;
		// sleep( 1000 ) ;
		vec.push_back(conn) ;
	}
	for(int i = 0;i != vec.size();++i)
	{
		ds.releaseConnection(vec[i]) ;
	}
	printf( "before sleep, datasource idle connection num: %d\n", ds.getIdleConnNum() ) ;
	ossSleep( 6*1000 ) ;
	printf( "after sleep, datasource idle connection num: %d\n", ds.getIdleConnNum() ) ;
	ASSERT_EQ(ds.getIdleConnNum(),20) ;
	ds.close() ;
}

/*
// 休眠时间<checkInterval,检查连接池空闲连接数量> maxIdleConnNum
TEST(TimeTest,checkIntervalShort)
{
	getConf() ;
	sdbDataSourceConf conf ;
	string url = COORD ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
	vector<sdb*> vec ;

	conf.setCheckIntervalInfo(3000,0) ;
	ds.init(url,conf) ;
	ds.enable() ;
	while(vec.size() <= conf.getMaxIdleCount()+10)	// 获取连接数超过maxIdleConnNum
	{
		ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;
		vec.push_back(conn) ;
	}
	cout<<"vector of connection size is "<<vec.size()<<endl ;
	for(int i = 0;i != vec.size();++i)			// 将这些连接释放回连接池
	{
		ds.releaseConnection(vec[i]) ;
	}
	int connNum = ds.getIdleConnNum() ;			// 不休眠,空闲队列连接数应>=maxIdleConnNu
	cout<<"Idle connNum is "<<connNum<<endl ;	
	ASSERT_GE(connNum,20) ;
	ds.close() ;
}


// 设置keepAliveTimeout=0,检查连接有效性
TEST(TimeTest,keepAliveTimoutZero)
{
	getConf() ;
	sdbDataSourceConf conf ;
	string url = COORD ;
	sdbDataSource ds ;
	sdb* conn = NULL ;

	conf.setCheckIntervalInfo(3000,0) ;
	ds.init(url,conf) ;
	ds.enable() ;
	ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;
	ASSERT_EQ(9,ds.getIdleConnNum()) ;
	ossSleep(3*1000) ;
	ds.releaseConnection(conn) ;
	ossSleep(3*1000) ;							// 休眠时间>checkInterval,但是keepAliveTimeout=0,该连接应该有效，空闲连接数为10
	ASSERT_EQ(10,ds.getIdleConnNum()) ;
	ds.close() ;
}


// 设置keepAliveTimeout!=0,检查连接有效性
TEST(TimeTest,keepAliveTimoutNotZero)
{
	getConf() ;
	sdbDataSourceConf conf ;
	string url = COORD ;
	sdbDataSource ds ;
	sdb* conn = NULL ;

	conf.setCheckIntervalInfo(3000,6000) ;
	ds.init(url,conf) ;
	ds.enable() ;
	ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;
	ASSERT_EQ(9,ds.getIdleConnNum()) ;
	ossSleep(7*1000) ;					// 获取连接后休眠时间>keepAliveTimeout，连接超时，此时连接池里的连接也都超时了。
	ds.releaseConnection(conn) ;
	ossSleep(4*1000) ;					// 释放连接后休眠时间>checkInterval,连接应该被清除，连接池的空闲连接为0
	ASSERT_EQ(0,ds.getIdleConnNum()) ;
	ds.close() ;
}

*/
/*
// 设置keepAliveTimeout!=0,检查连接有效性
TEST(TimeTest,keepAliveTimoutNotZeroAgain)
{
	getConf() ;
	sdbDataSourceConf conf ;
	string url = COORD ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
	sdbCollectionSpace cs ;

	conf.setCheckIntervalInfo(3000, 9000) ;
	ds.init(url,conf) ;
	ds.enable() ;
	ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;
        
	ASSERT_EQ(9, ds.getIdleConnNum()) ;   
	ossSleep(3*1000) ; 
	ASSERT_EQ(9, ds.getIdleConnNum()) ;
	ossSleep(1*1000) ; 
	ASSERT_EQ(9, ds.getIdleConnNum()) ;
	ossSleep(1*1000) ;
	ASSERT_EQ(9, ds.getIdleConnNum()) ;   
	ossSleep(2*1000) ; 
	ASSERT_EQ(0, ds.getIdleConnNum()) ;
	
	conn->createCollectionSpace( "datasourceTestCs_lxw",SDB_PAGESIZE_4K,cs ) ;
	conn->dropCollectionSpace( "datasourceTestCs_lxw" ) ;
	ds.releaseConnection(conn) ;
	ASSERT_EQ(1,ds.getIdleConnNum()) ;
	ossSleep(10*1000) ;
	ASSERT_EQ(0, ds.getIdleConnNum()) ;
	ds.close() ;
}
*/

/* 手动测试
// 出池检验连接有效性，停节点后应获取不到连接
TEST(ValidateTest,trueTest)
{
	sdbDataSourceConf conf ;
	string url = "localhost:11910" ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
	conf.setValidateConnection(true) ;
	conf.setSyncCoordInterval(false) ;
	ASSERT_EQ(SDB_OK,ds.init(url,conf)) ;		
	ASSERT_EQ(SDB_OK,ds.enable()) ;
	ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;

	sdb temp ;
	ASSERT_EQ(SDB_OK,temp.connect("192.168.31.61",11920)) ;
	sdbReplicaGroup group ;
	ASSERT_EQ(SDB_OK,temp.getReplicaGroup(2,group)) ;
	int nodeNum = 0 ;
	ASSERT_EQ(SDB_OK,group.getNodeNum(SDB_NODE_ALL,&nodeNum)) ;
	if(nodeNum < 2)
		return ;
	sdbReplicaNode node ;
	ASSERT_EQ(SDB_OK,group.getNode("sdbserver1","11910",node)) ;
	ASSERT_EQ(SDB_OK,node.stop()) ;
	//ASSERT_EQ(SDB_NETWORK,temp.connect("192.168.31.61",11910)) ;
	//ASSERT_EQ(0,conn->isValid()) ;
	cout << "before" << endl;
	while( 1 == conn->isValid() )
		ossSleep(1000) ;
	cout << "end" << endl;
	ASSERT_EQ(SDB_DS_NO_COORD,ds.getConnection(conn)) ;
	ds.close() ;
	ASSERT_EQ(SDB_OK,temp.connect("192.168.31.61",11920)) ;
	ASSERT_EQ(SDB_OK,node.start()) ;
}

// 出池不检验连接有效性，停节点能获得连接但连接无效？
TEST(ValidateTest,falseTest)
{
	sdbDataSourceConf conf ;
	string url = "192.168.31.61:11910" ;
	sdbDataSource ds ;
	sdb* conn = NULL ;
	conf.setValidateConnection(true) ;
	conf.setSyncCoordInterval(false) ;
	ASSERT_EQ(SDB_OK,ds.init(url,conf)) ;		
	ASSERT_EQ(SDB_OK,ds.enable()) ;
	ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;

	sdb temp ;
	ASSERT_EQ(SDB_OK,temp.connect("192.168.31.61",11920)) ;
	sdbReplicaGroup group ;
	ASSERT_EQ(SDB_OK,temp.getReplicaGroup(2,group)) ;
	int nodeNum = 0 ;
	ASSERT_EQ(SDB_OK,group.getNodeNum(SDB_NODE_ALL,&nodeNum)) ;
	if(nodeNum < 2)
		return ;
	sdbReplicaNode node ;
	ASSERT_EQ(SDB_OK,group.getNode("sdbserver1","11910",node)) ;
	ASSERT_EQ(SDB_OK,node.stop()) ;
	//ASSERT_EQ(SDB_NETWORK,temp.connect("192.168.31.61",11910)) ;
	//ASSERT_EQ(0,conn->isValid()) ;
	cout << "before" << endl;
	while( 1 == conn->isValid() )
		ossSleep(1000) ;
	cout << "end" << endl ;
	ASSERT_EQ(SDB_DS_NO_COORD,ds.getConnection(conn)) ;
	//ASSERT_EQ(SDB_OK,ds.getConnection(conn)) ;
	//ASSERT_EQ(0,conn->isValid()) ;
	ds.close() ;
	ASSERT_EQ(SDB_OK,temp.connect("192.168.31.61",11920)) ;
	ASSERT_EQ(SDB_OK,node.start()) ;
}
*/
