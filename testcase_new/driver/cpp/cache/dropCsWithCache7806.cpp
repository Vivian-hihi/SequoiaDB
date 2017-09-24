/**************************************************************
 * @Description: turn on cache and drop cs
 *               seqDB-7806 : turn on cache and drop cs by another connection
 * @Modify     : Suqiang Ling
 *               2017-09-11
 ***************************************************************/
#include <gtest/gtest.h>
#include <client.hpp>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include "testcommon.hpp"
#include "arguments.hpp"
#include "testBase.hpp"
#include "client.hpp"

using namespace sdbclient ;
using namespace bson ;
using namespace std ;

class turnOnCache7806 : public testBase 
{
protected:
   const CHAR *pCsName ;

   void SetUp() 
   {
      INT32 rc = SDB_OK ;

      // init client
      sdbClientConf conf ;
      conf.enableCacheStrategy = TRUE ;
      conf.cacheTimeInterval = 0 ;
      rc = initClient( &conf );
      ASSERT_EQ( SDB_OK, rc ) << "fail to initClient" ;

      // connect and create cs
      pCsName = "turnOnCache7806" ;
      sdbCollectionSpace cs ;
      rc = db.connect( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd() );
      ASSERT_EQ( SDB_OK, rc ) << "fail to connect db" ;
      rc = db.createCollectionSpace( pCsName, SDB_PAGESIZE_4K, cs ) ;
      ASSERT_EQ( SDB_OK, rc ) << "fail to create cs" ;
   }

   void TearDown() 
   {
      db.disconnect() ;
   }
} ;

TEST_F( turnOnCache7806, dropCollectionSpace )
{
   INT32 rc = SDB_OK ;
   sdb db2 ; // another db to drop cs
   rc = db2.connect( ARGS->hostName(), ARGS->svcName(), ARGS->user(), ARGS->passwd() );
   ASSERT_EQ( SDB_OK, rc ) << "fail to connect db2" ;
   rc = db2.dropCollectionSpace( pCsName ) ; 
   ASSERT_EQ( SDB_OK, rc ) << "fail to drop cs" ;
   db2.disconnect() ;

   ossSleep( 0 ) ; // sleep utill cs is not in cache. // TODO: i don't know the time length.
   sdbCollectionSpace cs ;
   rc = db.getCollectionSpace( pCsName, cs ) ;
   //ASSERT_EQ( SDB_DMS_CS_NOTEXIST, rc );
}
