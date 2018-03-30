/********************************************************
 * @Description: test case for c++ driver
 *               seqDB-14878:更新删除节点配置
 * @Modify:      Liangxw
 *               2018-03-30
 ********************************************************/
#include <client.hpp>
#include <gtest/gtest.h>
#include "testcommon.hpp"
#include "testBase.hpp"
#include "arguments.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

class confTest14878 : public testBase
{
protected:
   CHAR host[ MAX_NAME_SIZE+1 ] ;
   CHAR svc[ MAX_NAME_SIZE+1 ] ;
   
   void SetUp()
   {
      testBase::SetUp() ;
      memset( host, 0, MAX_NAME_SIZE+1 ) ;
      memset( svc, 0, MAX_NAME_SIZE+1 ) ;
   }
   void TearDown()
   {
      testBase::TearDown() ;
   }

   INT32 init()
   {
      INT32 rc = SDB_OK ;
      vector<string> groups ;
      vector<string> nodes ;
      const CHAR* rgName ;
      string nodename ;
      string hostname ;
      string svcname ;
      INT32 pos ;

      rc = getGroups( db, groups ) ;
      CHECK_RC( SDB_OK, rc, "fail to get groups" ) ;
      rgName = groups[0].c_str() ;
      rc = getGroupNodes( db, rgName, nodes ) ;
      CHECK_RC( SDB_OK, rc, "fail to group nodes" ) ;
      pos = nodes[0].find_first_of( ":" ) ;
      hostname = nodes[0].substr( 0, pos ) ;
      svcname = nodes[0].substr( pos+1 ) ;
      strcpy( host, hostname.c_str() ) ;
      strcpy( svc, svcname.c_str() ) ;
      printf( "node: %s:%s\n", host, svc ) ;

   done:
      return rc ;
   error:
      goto done ;
   }

   INT32 getNodeWeight( INT32* weight )
   {
      sdbCursor cursor ;
      INT32 rc = SDB_OK ;
      BSONObj obj ;
   
      BSONObj cond = BSON( "HostName" << host << "svcname" << svc ) ;
      rc = db.getSnapshot( cursor, SDB_SNAP_CONFIGS, cond ) ;
      CHECK_RC( SDB_OK, rc, "fail to snapshot conf" ) ;
   
      rc = cursor.next( obj ) ;
      CHECK_RC( SDB_OK, rc, "fail to get next" ) ;
      *weight = obj.getField( "weight" ).Int() ;
      
      rc = cursor.close() ;
      CHECK_RC( SDB_OK, rc, "fail to close cursor" ) ;
   done:
      return rc ;
   error:
      goto done ;
   }
} ;

TEST_F( confTest14878, upDelConf )
{
   if( isStandalone( db ) )
   {
      printf( "Run mode is standalone" ) ;
      return ;
   }
   INT32 rc = SDB_OK ;
   rc = init() ;
   ASSERT_EQ( SDB_OK, rc ) ;
   
   // update weight to 20 on data node
   BSONObj config = BSON( "weight" << 20 ) ;
   BSONObj option = BSON( "HostName" << host << "svcname" << svc ) ;
   rc = db.updateConfig( config, option ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to update conf" ;

   // check update
   INT32 weight = 0 ;
   rc = getNodeWeight( &weight ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( 20, weight ) << "fail to check update" ;

   // delete weight
   config = BSON( "weight" << 1 ) ;
   rc = db.deleteConfig( config, option ) ;
   ASSERT_EQ( SDB_OK, rc ) << "fail to delete conf" ;

   // check delete
   rc = getNodeWeight( &weight ) ;
   ASSERT_EQ( SDB_OK, rc ) ;
   ASSERT_EQ( 10, weight ) << "fail to check delete" ; 
}
