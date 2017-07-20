/**************************************************************
* @Description: test case for Jira questionaire
*				SEQUOIADBMAINSTREAM-1958
*				test ssl when usessl is true in configure file
*				sequoiadb need to be enterprise
* @Modify:      Liang xuewang Init
*               2017-07-19
**************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include "../testcommon.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

const char* CsModName = "sslTestCs" ;
char CsName[100] ;
const char* ClName = "sslTestCl" ;

// 测试开启ssl，sdb( useSSL=true )
TEST( sslTrue, sdbTrue )
{
	int rc = SDB_OK ;

	// 使用ssl连接sdb并创建集合空间、集合
	getConf() ;
	sdb db( true ) ;
	rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
	ASSERT_EQ( SDB_OK, rc ) << "fail to connect sdb when ssl is open" ;

	getUniqueName( CsModName, CsName ) ;
	sdbCollectionSpace cs ;
	rc = db.createCollectionSpace( CsName, SDB_PAGESIZE_4K, cs ) ;
	ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << CsName ;

	sdbCollection cl ;
	rc = cs.createCollection( ClName, cl ) ;
	ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << ClName ;

	// 删除集合空间、断开连接
	rc = db.dropCollectionSpace( CsName ) ;
	ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << CsName ;
	
	db.disconnect() ;
}

// 测试开启ssl，sdb( useSSL=false )
TEST( sslTrue, sdbFalse )          
{                          
    int rc = SDB_OK ;
               
    // 使用ssl连接sdb并创建集合空间、集合
    getConf() ;
    sdb db ;
    rc = db.connect( HOSTNAME, SVCNAME, USER, PASSWD ) ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to connect sdb when ssl is open" ;
          
    getUniqueName( CsModName, CsName ) ;
    sdbCollectionSpace cs ;
    rc = db.createCollectionSpace( CsName, SDB_PAGESIZE_4K, cs ) ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << CsName ;
    
    sdbCollection cl ;
    rc = cs.createCollection( ClName, cl ) ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << ClName ;
    
    // 删除集合空间、断开连接
    rc = db.dropCollectionSpace( CsName ) ;
    ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << CsName ;
    
    db.disconnect() ;
}
