/**************************************************************
 * @Description: test case for Jira questionaire
 *				     SEQUOIADBMAINSTREAM-1958
 *				     test ssl with ssl is allowed in configure file
 * @Modify:      Liang xuewang Init
 *               2016-11-10
 **************************************************************/
#include <gtest/gtest.h>
#include <client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "testcommon.hpp"
#include "arguments.hpp"

const CHAR* csName = "sslTestCs" ;
const CHAR* clName = "sslTestCl" ;
sdbConnectionHandle db ;
sdbCSHandle cs ;
sdbCollectionHandle cl ;

// 测试开启ssl，使用sdbSecureConnect正常连接创建集合空间、集合
TEST( sslTrue, sdbSecureConnect )
{
   INT32 rc = SDB_OK ;

   // 使用ssl连接sdb并创建集合空间、集合
   rc = sdbSecureConnect( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd(), &db ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to secure connect when ssl is open" ;
   rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, &cs ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << csName ;
   rc = sdbCreateCollection( cs, clName, &cl ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << clName ;

   // 删除集合、集合空间、断开连接
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to drop cl " << clName ;

   rc = sdbDropCollectionSpace( db, csName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << csName ;

   sdbDisconnect( db ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
}

// 测试开启ssl，使用sdbSecureConnect1正常连接创建集合空间、集合
TEST( sslTrue, sdbSecureConnect1 )
{
   INT32 rc = SDB_OK ;

   // 使用ssl连接sdb并创建集合空间、集合
   CHAR connAddr[200] ;
   sprintf( connAddr, "%s:%s", ARGS->hostName(), ARGS->svcName() ) ;
   const CHAR* connAddrs[1] ;
   connAddrs[0] = connAddr ;
   INT32 arrSize = sizeof(connAddrs) / sizeof(connAddrs[0]) ;
   rc = sdbSecureConnect1( connAddrs, arrSize, ARGS->user(), ARGS->passwd(), &db ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to secure connect when ssl is open" ;

   rc = sdbCreateCollectionSpace( db, csName, SDB_PAGESIZE_4K, &cs ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cs " << csName ;

   rc = sdbCreateCollection( cs, clName, &cl ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to create cl " << clName ;

   // 删除集合、集合空间、断开连接
   rc = sdbDropCollection( cs, clName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to drop cl " << clName ;
   rc = sdbDropCollectionSpace( db, csName ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs " << csName ;

   sdbDisconnect( db ) ;
   sdbReleaseCollection( cl ) ;
   sdbReleaseCS( cs ) ;
   sdbReleaseConnection( db ) ;
} 
