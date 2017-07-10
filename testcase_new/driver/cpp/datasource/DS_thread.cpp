#include "DS_thread.hpp"
#include "DS_common.hpp"
#include <ctime>

int getRand()
{
	static bool init = true ;
	if(init)
	{
		srand(time(NULL)) ;
		init = false ;
	}
	return rand()%10 ;
}

void init(DsArgs *arg)
{
	ossSleep(getRand()*100) ;
	sdbclient::sdbDataSourceConf conf ;
	getConf() ;
    std::string url = COORD ;
	ASSERT_EQ(SDB_OK,arg->getDs().init(url,conf)) ;
}

void init_enable(DsArgs *arg)
{
	ossSleep(getRand()*100) ;
	sdbclient::sdbDataSourceConf conf ;
	getConf() ;
    std::string url = COORD ;
	ASSERT_EQ(SDB_OK,arg->getDs().init(url,conf)) ;
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
}

void init_disable(DsArgs *arg)
{
	ossSleep(getRand()*100) ;
	sdbclient::sdbDataSourceConf conf ;
	getConf() ;
    std::string url = COORD ;
	ASSERT_EQ(SDB_OK,arg->getDs().init(url,conf)) ;
	ASSERT_EQ(SDB_OK,arg->getDs().disable()) ;
}

void init_close(DsArgs *arg)
{
	ossSleep(getRand()*100) ;
	sdbclient::sdbDataSourceConf conf ;
	getConf() ;
    std::string url = COORD ;
	ASSERT_EQ(SDB_OK,arg->getDs().init(url,conf)) ;
	arg->getDs().close() ;
}

void init_conn(DsArgs *arg)
{
	ossSleep(getRand()*100) ;
	sdbclient::sdbDataSourceConf conf ;
	getConf() ;
    std::string url = COORD ;
	ASSERT_EQ(SDB_OK,arg->getDs().init(url,conf)) ;
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
	sdbclient::sdb* conn = NULL ;
	ASSERT_EQ(SDB_OK,arg->getDs().getConnection(conn)) ;
	arg->getDs().releaseConnection(conn) ;
}

void init_coord(DsArgs *arg)
{
	ossSleep(getRand()*100) ;
	sdbclient::sdbDataSourceConf conf ;
	getConf() ;
    std::string url = COORD ;
	ASSERT_EQ(SDB_OK,arg->getDs().init(url,conf)) ;
	std::string url2 = "localhost:11910" ;
	arg->getDs().addCoord(url2) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url2) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
}

void enable(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
}

void enable_disable(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
	ASSERT_EQ(SDB_OK,arg->getDs().disable()) ;
}

void enable_close(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
	arg->getDs().close() ;
}

void enable_conn(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
	sdbclient::sdb *conn = NULL ;
	ASSERT_EQ(SDB_OK,arg->getDs().getConnection(conn)) ;
	arg->getDs().releaseConnection(conn) ;
}

void enable_coord(DsArgs *arg)
{
	ASSERT_EQ(SDB_OK,arg->getDs().enable()) ;
	std::string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
}

void disable(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().disable()) ;
}

void disable_close(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().disable()) ;
	arg->getDs().close() ;
}

void disable_conn(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().disable()) ;
	sdbclient::sdb *conn = NULL ;
	ASSERT_EQ(SDB_DS_NOT_ENABLE,arg->getDs().getConnection(conn)) ;
	arg->getDs().releaseConnection(conn) ;
}

void disable_coord(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	ASSERT_EQ(SDB_OK,arg->getDs().disable()) ;
	std::string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
}

void dsclose(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	arg->getDs().close() ;
}

void dsclose_conn(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	sdbclient::sdb *conn = NULL ;
	ASSERT_EQ(SDB_OK,arg->getDs().getConnection(conn)) ;
	arg->getDs().releaseConnection(conn) ;
	arg->getDs().close() ;
}

void dsclose_coord(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	std::string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().close() ;
}

void connection(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	sdbclient::sdb *conn = NULL ;
	ASSERT_EQ(SDB_OK,arg->getDs().getConnection(conn)) ;
	arg->getDs().releaseConnection(conn) ;
}


void connection_coord(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	sdbclient::sdb *conn = NULL ;
	ASSERT_EQ(SDB_OK,arg->getDs().getConnection(conn)) ;
	arg->getDs().releaseConnection(conn) ;
	std::string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
}

void releaseConn(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	std::vector<sdbclient::sdb *> vec = arg->getConnVec() ;
	for(int i = 0;i != vec.size();++i)
		arg->getDs().releaseConnection(vec[i]) ;
}

void releaseConn_coord(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	std::vector<sdbclient::sdb *> vec = arg->getConnVec() ;
	for(int i = 0;i != vec.size();++i)
		arg->getDs().releaseConnection(vec[i]) ;
	string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
}

void addCoord(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_EQ(2,arg->getDs().getNormalCoordNum()) ;
}

void addCoord_remove(DsArgs *arg)
{
	string url = "localhost:11910" ;
	arg->getDs().addCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
	arg->getDs().removeCoord(url) ;
	ASSERT_LE(arg->getDs().getNormalCoordNum(),2) ;
}

void removeCoord(DsArgs *arg)
{
	ossSleep(getRand() * 100) ;
	getConf() ;
	string url = COORD ;
	arg->getDs().removeCoord(url) ;
	ASSERT_EQ(0,arg->getDs().getNormalCoordNum()) ;
}
