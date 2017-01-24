/**************************************************************
* @Description: test case for Jira questionaire Task
*				SEQUOIADBMAINSTREAM-2165
*				seqDB-11001:reloadConf
*               手工测试：修改数据节点的配置文件 diaglevel
*				指定日志级别后调用reloadConf,检查节点日志
* @Modify     : Liang xuewang Init
*			 	2017-01-22
***************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include "../common/testcommon.hpp"

sdbConnectionHandle db   = SDB_INVALID_HANDLE ;
// 修改协调节点配置文件如/lxw/trunk/conf/local/50000/sdb.conf diaglevel=4 默认为3
// 协调节点HOSTNAME:SVCNAME

TEST( reloadConf, indexpath )
{
	INT32 rc = SDB_OK ;

    // connect to sdb
	getConf() ;
	rc = sdbConnect( HOSTNAME, SVCNAME, USER, PASSWD, &db ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to connect sdb" ;

	// reload conf
	bson option ;
	bson_init( &option ) ;
	bson_append_bool( &option, "Global", false ) ;
	bson_finish( &option ) ;
	rc = sdbReloadConfig( db, &option ) ;
	bson_destroy( &option ) ;
	ASSERT_EQ( rc, SDB_OK ) << "fail to reload conf" ;

	// 断开连接后检查节点日志中是否有INFO
	sdbDisconnect( db ) ;
	sdbReleaseConnection( db ) ;
}
